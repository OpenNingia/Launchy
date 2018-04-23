/*
Launchy: Application Launcher
Copyright (C) 2007-2010  Josh Karlin, Simon Capewell

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef CATALOG_BUILDER
#define CATALOG_BUILDER


#include <QThread>
#include <QRunnable>
#include "catalog_types.h"
#include "plugin_handler.h"


#define CATALOG_PROGRESS_MIN 0
#define CATALOG_PROGRESS_MAX 100

class CatalogAdder : public QThread
{
    Q_OBJECT

public:
    CatalogAdder(Catalog& cat);
    void run();

public slots:
    void push(QList<CatItem> items);
    void finish();
    void abort();

private:
    QMutex m_mutex;
    QQueue<CatItem> m_queue;
    Catalog& m_cat;
    bool m_active;
    bool m_stop_requested;
    bool m_abort_requested;
};

class CatalogBuilder : public QThread, public INotifyProgressStep
{
	Q_OBJECT

public:
	CatalogBuilder(PluginHandler* plugs);
	Catalog* getCatalog() const { return catalog; }
	int getProgress() const { return progress; }
	bool progressStep(int newStep);
    void run();

public slots:
	void buildCatalog();
    void stop();

signals:
	void catalogIncrement(int);
	void catalogFinished();

private:
	void indexDirectory(const QString& dir, const QStringList& filters, bool fdirs, bool fbin, int depth);

	PluginHandler* plugins;
	Catalog* catalog;
    QSharedPointer<CatalogAdder> adder;
	QSet<QString> indexed;
	int progress;
	int currentItem;
	int totalItems;
    volatile bool stop_request;
};


#endif
