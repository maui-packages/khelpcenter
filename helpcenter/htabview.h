/*
 *  htabview.h - part of the KDE Help Center
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

#ifndef __htabview_h__
#define __htabview_h__

#include <qlist.h>

#include <ktreelist.h>
#include <ktabctl.h>

class IndexWidget;
class SearchWidget;
class HTreeListItem;

class HTabView : public KTabCtl
{
    Q_OBJECT
  
 public:
    HTabView(QWidget *parent=0, const char *name=0);
    virtual ~HTabView();

 public slots:
    void slotURLSelected(QString url);
    void slotItemSelected(int index);
    void slotReloadTree();
    void slotTabSelected(int);

 signals:
    void itemSelected(QString itemURL);
    void setBussy(bool bussy);
	
 private:
    void setupContentsTab();
    void setupIndexTab();
    void setupSearchTab();
    void buildTree();
    void clearTree();

    void buildManSubTree(HTreeListItem *parent);
    void buildManualSubTree(HTreeListItem *parent);
    void insertPlugins();

    bool appendEntries (const char *dirName,  HTreeListItem *parent, QList<HTreeListItem> *appendList);
    bool processDir(const char *dirName, HTreeListItem *parent,  QList<HTreeListItem> *appendList);
    bool containsDocuments(QString dir);

    KTreeList *tree;
    SearchWidget *search;
    IndexWidget *index;

    QList<HTreeListItem> staticItems, manualItems, pluginItems;
};

#endif
