/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: Network simulator. Emulates Zigbee network topology.
*/

#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

using namespace std;

int load_xgml(string xgml_path, map<int, vector<int> > &visibility_matrix)
{
  int ret = -1;
  xmlDocPtr doc = NULL;
  xmlXPathContextPtr xpathCtx = NULL;
  xmlXPathObjectPtr xpathObj = NULL;
  int i, size;

  cout << "[XGML parse begin]" << endl;

  if ( !xgml_path.length() )
  {
    goto done;
  }

  /* Init libxml */
  xmlInitParser();

  /* Load XML document */
  cout << "xgml file path: " << xgml_path << endl;
  doc = xmlParseFile(xgml_path.c_str());
  if (doc == NULL)
  {
    cout << "unable to parse file: " << xgml_path << endl;
    goto done;
  }

  /* Create xpath evaluation context */
  xpathCtx = xmlXPathNewContext(doc);
  if ( xpathCtx == NULL )
  {
    cout << "unable to create new XPath context" << endl;
    goto done;
  }

  /* Evaluate xpath expression */
  xpathObj = xmlXPathEvalExpression(BAD_CAST "//section[@name='node']/attribute[@key='id']", xpathCtx);
  if ( xpathObj == NULL )
  {
    cout << "unable to evaluate xpath expression" << endl;
    goto done;
  }

  size = (xpathObj->nodesetval) ? xpathObj->nodesetval->nodeNr : 0;
  cout << "xgml found " << size << " nodes" << endl;
  for (i = 0; i < size; i++)
  {
    xmlNodePtr cur = xpathObj->nodesetval->nodeTab[i];
    if ( cur->type == XML_ELEMENT_NODE )
    {
      cout << "node index: " << xmlNodeGetContent(cur) << endl;
    }
  }

  /* Cleanup */
  xmlXPathFreeObject(xpathObj);

  /* Evaluate xpath expression */
  xpathObj = xmlXPathEvalExpression(BAD_CAST "//section[@name='edge' and attribute/@key = 'source' and attribute/@key = 'target' ]", xpathCtx);
  size = (xpathObj->nodesetval) ? xpathObj->nodesetval->nodeNr : 0;
  cout << "xgml found " << size << " edges" << endl;
  for (i = 0; i < size; i++)
  {
    xmlNodePtr cur = xpathObj->nodesetval->nodeTab[i];
    if ( cur->type == XML_ELEMENT_NODE )
    {
      xmlXPathContextPtr subCtx = xmlXPathNewContext(doc);
      xmlXPathObjectPtr xpathObj_source;
      xmlXPathObjectPtr xpathObj_target;

      subCtx->node = cur;
      xpathObj_source = xmlXPathEvalExpression(BAD_CAST "attribute[@key = 'source']", subCtx);
      xpathObj_target = xmlXPathEvalExpression(BAD_CAST "attribute[@key = 'target']", subCtx);

      int s = (int)xmlXPathCastStringToNumber(xmlNodeGetContent(xpathObj_source->nodesetval->nodeTab[0]));
      int t = (int)xmlXPathCastStringToNumber(xmlNodeGetContent(xpathObj_target->nodesetval->nodeTab[0]));
      cout << "edge: " << s << " <--> " << t << endl;

      visibility_matrix[s].push_back(t);
      visibility_matrix[t].push_back(s);

      xmlXPathFreeObject(xpathObj_source);
      xmlXPathFreeObject(xpathObj_target);
      xmlXPathFreeContext(subCtx);
    }
  }

  ret = 0;
  done:
  /* Cleanup */
  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);
  xmlFreeDoc(doc);

  /* Shutdown libxml */
  xmlCleanupParser();

  cout << ( ret ? "error" : "success" ) << endl;
  cout << "[XGML parse end]" << endl;
  return ret;
}
