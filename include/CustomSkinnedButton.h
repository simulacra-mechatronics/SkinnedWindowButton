#ifndef CUSTOMSKINNEDBUTTON_H
#define CUSTOMSKINNEDBUTTON_H

#include <windows.h>
#include "BitmapSkin.h"

#define BUTTONSTATE_OUT 0
#define BUTTONSTATE_OVER 1
#define BUTTONSTATE_IN 2

class CustomSkinnedButton
{
    public:
        CustomSkinnedButton(HINSTANCE hInstance, WORD wBitmapResource_ButtonOut, WORD wBitmapResource_ButtonOver, WORD wBitmapResource_ButtonIn, COLORREF transparencyColorKey);
        void mouseIsOver();
        void mouseIsOut();
        void mouseLeftDown();
        void mouseLeftUp();
        bool isMouseOver();
        bool isMouseLeftButtonDown();
        LONG getBitmapWidth();
        LONG getBitmapHeight();
        WORD getbuttonState();
        HBITMAP getHandleButtonOutBitmap();
        HBITMAP getHandleButtonOverBitmap();
        HBITMAP getHandleButtonInBitmap();
        HRGN getHandleToVisibleRegion();

        ~CustomSkinnedButton();

    private:
        void SetVisibleRgnFromBitmap(HBITMAP hBmp, COLORREF color);

        bool bMouseOver = false;                // A boolean flag set to signal that the mouse is hovering over the button
        bool bMouseLeftButtonDown = false;      // A boolean flag set to signal that the left mouse button was clicked while over the button
        HINSTANCE hInstance = NULL;             // A handle to the instance of the module whose executable file contains the bitmap to be loaded

        LONG bmWidth;
        LONG bmHeight;

        HRGN hVisibleRegion;

        BitmapSkin* pButtonIn;
        BitmapSkin* pButtonOver;
        BitmapSkin* pButtonOut;
};

#endif // CUSTOMSKINNEDBUTTON_H
