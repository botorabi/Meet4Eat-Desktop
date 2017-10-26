/**
 * Copyright (c) 2017 by Botorabi. All rights reserved.
 * https://github.com/botorabi/Meet4Eat
 *
 * License: MIT License (MIT), read the LICENSE text in
 *          main directory for more details.
 */

#ifndef GUIUTILS_H
#define GUIUTILS_H

#include <configuration.h>
#include <document/modeldocument.h>
#include <webapp/webapp.h>
#include <QPixmap>
#include <QWidget>


namespace m4e
{
namespace common
{

/**
 * @brief A collection of UI utilities
 *
 * @author boto
 * @date Sep 19, 2017
 */
class GuiUtils
{
    public:

        /**
         * @brief Given an input image, create a round icon out of it.
         *
         * @param input Input image
         * @return      Round icon
         */
        static QPixmap      createRoundIcon( const QPixmap& input );

        /**
         * @brief createRoundIcon
         * @brief Given an image document, create a round icon out of it.
         *
         * @param input Input image document
         * @return      Round icon
         */
        static QPixmap      createRoundIcon( doc::ModelDocumentPtr input );

        /**
         * @brief Create an image in proper size by letting the user choos an image file. The image has a max size of 512x512.
         *
         * @param p_parent      Parent widget
         * @param dir           Directory to search for files, let empty to take the system default for documents directory
         * @param image         If successfull the image pixmap will be stored here
         * @param imageContent  If successfull the image content will be stored here
         * @param format        Image format such as "png"
         * @param aborted       If the user aborted the image loading from file then 'aborted' will be set to true
         * @return              Return true if successful, false if the user has selected an unsupported image file format
         */
        static bool         createImageFromFile( QWidget* p_parent, QString dir, QPixmap& image, QByteArray& imageContent, QString& format, bool& aborted );

        /**
         * @brief Create and assign a ShadowEffect to given widget.
         *
         * @param p_widget Widget getting the effect
         * @param color    Shadow color
         * @param offset   Offset
         * @param blurr    Blurr radius
         */
        static void         createShadowEffect( QWidget* p_widget,
                                                const QColor& color = QColor( 100, 100, 100, 180 ),
                                                const QPoint& offset = QPoint( -2, 2 ),
                                                int blurr = 6 );

        /**
         * @brief Check if the given owner ID matches to currently authorized user's ID.
         *
         * @param ownerId   Owner ID to check
         * @param p_webApp  Web app interface containing information about the authorized user
         * @return          Return true if user is the owner, otherwise false.
         */
        static bool         userIsOwner( const QString& ownerId, webapp::WebApp* p_webApp );
};

} // namespace common
} // namespace m4e

#endif // GUIUTILS_H
