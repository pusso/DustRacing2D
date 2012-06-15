// This file is part of Dust Racing (DustRAC).
// Copyright (C) 2011 Jussi Lind <jussi.lind@iki.fi>
//
// DustRAC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// DustRAC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DustRAC. If not, see <http://www.gnu.org/licenses/>.

#ifndef EDITORVIEW_HPP
#define EDITORVIEW_HPP

#include <QGraphicsView>
#include <QMenu>
#include "../common/tracktilebase.hpp"

class QAction;
class QMouseEvent;
class QPaintEvent;
class EditorData;
class Object;
class ObjectLoader;
class TrackTile;

class EditorView : public QGraphicsView
{
    Q_OBJECT

public:

    explicit EditorView(QWidget * parent = 0);

protected:

    //! \reimp
    void mouseMoveEvent(QMouseEvent * event);

    //! \reimp
    void mousePressEvent(QMouseEvent * event);

    //! \reimp
    void mouseReleaseEvent(QMouseEvent * event);

private slots:

    void doRotateTile90CW();
    void doRotateTile90CCW();
    void doRotateObject();
    void doClearComputerHint();
    void doSetComputerHintFirstBeforeCorner();
    void doSetComputerHintSecondBeforeCorner();
    void doClearDrivingLineHintH();
    void doClearDrivingLineHintV();
    void doSetDrivingLineHintLeft();
    void doSetDrivingLineHintRight();
    void doSetDrivingLineHintTop();
    void doSetDrivingLineHintBottom();

private:

    void createTileContextMenu();
    void createObjectContextMenu();
    void handleLeftButtonClickOnTile(TrackTile & tile);
    void handleRightButtonClickOnTile(TrackTile & tile);
    void handleLeftButtonClickOnObject(Object & object);
    void handleRightButtonClickOnObject(Object & object);
    void handleTileDragRelease(QMouseEvent * event);
    void handleObjectDragRelease(QMouseEvent * event);
    void doSetDrivingLineHintH(TrackTileBase::DrivingLineHintH hint);
    void doSetDrivingLineHintV(TrackTileBase::DrivingLineHintV hint);
    void doSetComputerHint(TrackTileBase::ComputerHint hint);

    QMenu     m_tileContextMenu;
    QMenu     m_objectContextMenu;
    QPoint    m_clickedPos;
    QPointF   m_clickedScenePos;
    QAction * m_clearComputerHint;
    QAction * m_setComputerHintFirstBeforeCorner;
    QAction * m_setComputerHintSecondBeforeCorner;
    QAction * m_clearDrivingLineHintH;
    QAction * m_clearDrivingLineHintV;
    QAction * m_setDrivingLineHintLeft;
    QAction * m_setDrivingLineHintRight;
    QAction * m_setDrivingLineHintTop;
    QAction * m_setDrivingLineHintBottom;
};

#endif // EDITORVIEW_HPP
