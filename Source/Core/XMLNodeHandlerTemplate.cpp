/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "precompiled.h"
#include "XMLNodeHandlerTemplate.h"
#include "Template.h"
#include "TemplateCache.h"
#include "XMLParseTools.h"
#include "../../Include/Rocket/Core.h"

namespace Rocket {
namespace Core {

XMLNodeHandlerTemplate::XMLNodeHandlerTemplate()
{
}

XMLNodeHandlerTemplate::~XMLNodeHandlerTemplate()
{
}

Element* XMLNodeHandlerTemplate::ElementStart(XMLParser* parser, const String& name, const XMLAttributes& attributes)
{
	// Tell the parser to use the element handler for all child nodes
	parser->PushDefaultHandler();

	Element* parent = parser->GetParseFrame()->element;

	// Attempt to instance the element with the instancer
	templateElement = Factory::InstanceElement(parent, name, name, attributes);
	if (!templateElement)
	{
		Log::Message(Log::LT_ERROR, "Failed to create element for tag %s, instancer returned NULL.", name.CString());
		return NULL;
	}

	// Add the element to its parent (remove the reference to templateElement in ElementEnd)
	parent->AppendChild(templateElement);

	//use the templateElement as a temporary container to hold the template content blocks
	return templateElement;
}

bool XMLNodeHandlerTemplate::ElementEnd(XMLParser* parser, const String& ROCKET_UNUSED(name))
{
	Element *parent = templateElement->GetParentNode();
	String template_name = templateElement->GetAttribute<String>("src", "");

	//find all content block elements from document
	ElementList docBlockElements;
	templateElement->GetElementsByTagName(docBlockElements, "block");

	//we are done with the templateElement, remove it and its children
	templateElement->GetParentNode()->RemoveChild(templateElement);
	templateElement->RemoveReference();

	//parse and insert template tree into parent
	Element *tree = XMLParseTools::ParseTemplate(parent, template_name);

	//find all the block elements from loaded template (default content)
	ElementList templateBlockElements;
	tree->GetElementsByTagName(templateBlockElements, "block");

	for (ElementList::iterator itr = templateBlockElements.begin(); itr != templateBlockElements.end(); ++itr)
	{
		Element *item = (*itr);
		String name = item->GetAttribute<String>("name","");

		for (ElementList::iterator itrC = docBlockElements.begin(); itrC != docBlockElements.end(); ++itrC)
		{
			String itemName = (*itrC)->GetAttribute<String>("name","");

			if( name == itemName)
			{
				//it's a match, now replace the default contents of this node
				//shouldn't have more than one child (default content)
				while( item->HasChildNodes())
				{
					item->RemoveChild( item->GetFirstChild());
				}

				Element *itemC = (*itrC);
				for( int i=0; i<itemC->GetNumChildren(); i++)
				{
					item->AppendChild(itemC->GetChild(i));
				}
			}
		}
	}

	return true;
}

bool XMLNodeHandlerTemplate::ElementData(XMLParser* parser, const String& data)
{	
	return Factory::InstanceElementText(parser->GetParseFrame()->element, data);
}

void XMLNodeHandlerTemplate::Release()
{
	delete this;
}

}
}
