/*
 *  khc_infohierarchymaker.cc - part of the KDE Help Center
 *
 *  Copyright (C) 2001 Wojciech Smigaj (achu@klub.chip.pl)
 *
 *  khc_navigator.h - part of the KDE Help Center
 *
 *  Copyright (C) 1999 Matthias Elter (me@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "khc_infohierarchymaker.h"

#include "khc_infoconsts.h"

#include <list>
#include <functional>
#include <algorithm>

#include <kdebug.h>

struct isParent: std::binary_function<const khcInfoNode*, const khcInfoNode*, bool>
{ 
  bool operator()(const khcInfoNode* pPotentialChild, 
	 	  const khcInfoNode* pPotentialParent) const
  { 
    return pPotentialChild->m_sUp == pPotentialParent->m_sName;
  } 
};  

struct isNode: std::binary_function<const khcInfoNode*, QString, bool>
{
  bool operator()(const khcInfoNode* pNode, 
		  QString sName) const
  {
    return pNode->m_sName == sName;
  }
}; 

struct isTop: std::unary_function<const khcInfoNode*, bool>
{
  bool operator()(const khcInfoNode* pNode) const
  {
    return pNode->m_sUp.lower() == "(dir)";
  }
}; 

struct isFirstSibling: std::unary_function<const khcInfoNode*, bool>
{
  bool operator()(const khcInfoNode* pNode) const
  {
    return pNode->m_sPrev == pNode->m_sUp || pNode->m_sPrev.isEmpty(); 
  }
}; 

struct isNextSibling: std::binary_function<const khcInfoNode*, 
		                      const khcInfoNode*, bool>
{
  bool operator()(const khcInfoNode* pPrevSibling, 
		  const khcInfoNode* pNode) const
  {
    return pNode->m_sPrev == pPrevSibling->m_sName;
  }
}; 


khcInfoHierarchyMaker::khcInfoHierarchyMaker() :
  m_bIsWorking(false)
{
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(getSomeNodes()));
}

khcInfoHierarchyMaker::~khcInfoHierarchyMaker()
{
  clearNodesList();
}

void khcInfoHierarchyMaker::clearNodesList()
{
  for (std::list<khcInfoNode*>::iterator it = m_lNodes.begin(); it != m_lNodes.end(); )
  {
    std::list<khcInfoNode*>::iterator copyIt(it);
    it++;
    delete *copyIt;
    m_lNodes.erase(copyIt);
  }
}

void khcInfoHierarchyMaker::createHierarchy(uint key, QString topic, QString root)
{
  //  qDebug("--- createHierarchy ---");

  ASSERT(!topic.isEmpty());
   
  clearNodesList();

  m_infoReader.setTopic(topic);
  m_key = key;
  //  m_topic = topic;
  m_root = root;

  m_timer.start(0);
  m_bIsWorking = true;
}

void khcInfoHierarchyMaker::getSomeNodes()
{
  unsigned int i = 10;
  uint nResult;

  while (i)
  {
    khcInfoNode* pNode = new khcInfoNode;
    nResult = m_infoReader.getNextNode(pNode, 
				       RETRIEVE_NAME |
				       RETRIEVE_TITLE |
				       RETRIEVE_NEIGHBOURS);
    switch (nResult)
    {
    case ERR_NONE:
//        qDebug("getNextNode returned %d", nResult);
//        kdDebug() <<
//  	     "Name: " << pNode->m_sName <<
//  	     " Up: " << pNode->m_sUp <<
//  	     " Prev: " << pNode->m_sPrev <<
//  	     " Next: |" << pNode->m_sNext <<
//  	     "\nTitle: " << pNode->m_sTitle << endl;
      m_lNodes.push_back(pNode);
      break;
    case ERR_NO_MORE_NODES:
      {
	// qDebug("No more nodes");
	m_timer.stop();
	
	// this was for testing:   m_lNodes.erase((++(++m_lNodes.begin())));

	khcInfoNode* pTopNode;
	bool bResult = makeHierarchy(&pTopNode, m_root);
	// qDebug ("makeHierarchy returned %d", bResult);
	if (bResult)
	{
	  // pTopNode->dumpChildren(0);
	  emit hierarchyCreated(m_key, ERR_NONE, pTopNode);
	  //	delete pTopNode;
	  restoreChildren(pTopNode);
	}
	else
	  emit hierarchyCreated(m_key, ERR_NO_HIERARCHY, 0);
	
	m_bIsWorking = false;
	return;
      }
    default:
      kdWarning() << "getNextNode returned " << nResult << endl;
      delete pNode;
      m_timer.stop();
      emit hierarchyCreated(m_key, nResult, 0);
      m_bIsWorking = false;
      return;
    }

    i--;
  }
}

bool khcInfoHierarchyMaker::makeHierarchy(khcInfoNode** ppTopNode,
					  QString topNodeName)
{
  // qDebug("--- makeHierarchy ---");

  std::list<khcInfoNode*>::iterator topIt;

  if (topNodeName.isEmpty())
    topIt = find_if(m_lNodes.begin(), m_lNodes.end(), isTop());
  else
    topIt = find_if(m_lNodes.begin(), m_lNodes.end(),
		    bind2nd(isNode(), topNodeName));

  if (topIt == m_lNodes.end())
    return false;

  *ppTopNode = *topIt;

  m_lNodes.erase(topIt);

  if (findChildren(*ppTopNode))
    return true;
  else
  {
    restoreChildren(*ppTopNode);
    *ppTopNode = 0;
    return false;
  }
}

// Recursively removes nodes from a tree with root == pParentNode and adds
// them to m_lNodes.
void khcInfoHierarchyMaker::restoreChildren(khcInfoNode* pParentNode)
{
  ASSERT(pParentNode);

  std::list<khcInfoNode*>& L = pParentNode->m_lChildren;
  for (std::list<khcInfoNode*>::iterator it = L.begin(); it != L.end(); )
  {
    std::list<khcInfoNode*>::iterator copyIt(it);
    it++;
    restoreChildren(*copyIt);
    L.erase(copyIt);
  }
    
  // qDebug("Actual restoring a node...");
  m_lNodes.push_back(pParentNode);
}

bool khcInfoHierarchyMaker::findChildren(khcInfoNode* pParentNode)
{
  std::list<khcInfoNode*>::iterator itAfterChildren = 
    partition(m_lNodes.begin(), m_lNodes.end(), 
	      bind2nd(isParent(), pParentNode));
  std::list<khcInfoNode*>& L = pParentNode->m_lChildren;
  L.splice(L.begin(), m_lNodes, m_lNodes.begin(), itAfterChildren);
  for (std::list<khcInfoNode*>::iterator it = L.begin(); it != L.end(); ++it)
    if (!findChildren(*it))
      return false;

  return orderSiblings(pParentNode->m_lChildren);
}

bool khcInfoHierarchyMaker::orderSiblings(std::list<khcInfoNode*>& siblingsList)
{
  if (siblingsList.empty())
    return true;

  // Szukanie pierwszego elementu
  std::list<khcInfoNode*>::iterator itFirst = 
    find_if(siblingsList.begin(), siblingsList.end(), isFirstSibling());
  if (itFirst == siblingsList.end())
    // Nie znaleziono "pierwszego" elementu
  {
    kdWarning() << "First child of " << (*siblingsList.begin())->m_sUp <<
      " not found." << endl;
    return false;
  }
  siblingsList.splice(siblingsList.begin(), siblingsList, itFirst);

  std::list<khcInfoNode*>::iterator itStart = ++siblingsList.begin();
  while (itStart != siblingsList.end())
  {
    std::list<khcInfoNode*>::iterator itPrev = itStart;
    --itPrev;

    std::list<khcInfoNode*>::iterator itNext =
      find_if(itStart, siblingsList.end(),
	      bind1st(isNextSibling(), *itPrev));
    if (itNext == siblingsList.end())
      // Nie znaleziono nastepnego elementu
    {
      kdWarning() << "Next sibling of " << (*itPrev)->m_sName << " not found" << endl;
      return false;
    }

    siblingsList.splice(itStart, siblingsList, itNext);
    
    itStart = (++(++itPrev));
  }

  return true;
}


#include "khc_infohierarchymaker.moc"
