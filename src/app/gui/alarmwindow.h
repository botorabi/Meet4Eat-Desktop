/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#ifndef ALARMWINDOW_H
#define ALARMWINDOW_H

#include <configuration.h>
#include <common/basedialog.h>
#include <event/modelevent.h>
#include <QMainWindow>
#include <QMouseEvent>
#include <QTimer>


namespace Ui {
  class AlarmWindow;
}

namespace m4e
{
namespace gui
{

class MainWindow;

/**
 * @brief A window for wakeup alarm.
 *
 * @author boto
 * @date Nov 10, 2017
 */
class AlarmWindow : public QMainWindow
{
    Q_OBJECT

    public:

        /**
         * @brief Create a dialog instance.
         *
         * @param p_parent  Parent widget
         */
        explicit                    AlarmWindow( MainWindow* p_parent );

        /**
         * @brief Destroy the instance.
         */
        virtual                     ~AlarmWindow();

        /**
         * @brief Setup the window.
         *
         * @param event  The alarming event
         */
        void                        setupUI( event::ModelEventPtr event );

    protected slots:

        void                        onBtnDiscardClicked();

        void                        onBtnDisplayEventClicked();

        void                        onTimer();

    protected:

        void                        mousePressEvent( QMouseEvent* p_event );

        void                        mouseReleaseEvent( QMouseEvent* p_event );

        void                        mouseMoveEvent( QMouseEvent* p_event );

        void                        startAnimation();

        MainWindow*                 _p_mainWindow = nullptr;

        Ui::AlarmWindow*            _p_ui         = nullptr;

        QTimer*                     _p_timer      = nullptr;

        event::ModelEventPtr        _event;

        bool                        _dragging     = false;

        QPoint                      _draggingPos;
};

} // namespace gui
} // namespace m4e

#endif // ALARMWINDOW_H
