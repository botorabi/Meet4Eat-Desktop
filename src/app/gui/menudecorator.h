/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#ifndef MENUDECORATOR_H
#define MENUDECORATOR_H

#include <configuration.h>
#include <QMenu>


namespace m4e
{
namespace gui
{

/**
 * @brief Class for decorating a menu
 *
 * @author boto
 * @date March 15, 2018
 */
class MenuDecorator
{
    public:

        explicit        MenuDecorator( QMenu* p_menu );

        /**
         * @brief Add a separator to current menu position.
         */
        void            addMenuSeperator();

        /**
         * @brief Add a meu item.
         *
         * @param label Menu label
         * @param data  Manu data
         */
        void            addMenuItem( const QString& label, const QVariant& data );

        /**
         * @brief Add a checkbox item to menu.
         *
         * @param label     Menu label
         * @param data      Menu data
         * @param checked   The initial checkbox state
         */
        void            addMenuItemCheckbox( const QString& label, const QVariant& data, bool checked );

    protected:

        QMenu*          _p_menu = nullptr;
};

} // namespace gui
} // namespace m4e

#endif // MENUDECORATOR_H
