#ifndef BITMAPSKIN_H
#define BITMAPSKIN_H

#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

class BitmapSkin
{
    public:
        /** Default constructor */
        BitmapSkin(HINSTANCE hInstance, WORD wBitmapResourceName);              // Constructor
        bool loadFromResource(HINSTANCE hInstance, WORD wBitmapResourceName);   // Loads a bitmap from a resource file
        HBITMAP getBitmapHandle();                                              // Returns the Bitmap handle
        LONG getBitmapWidth();
        LONG getBitmapHeight();

        /** Default destructor */
        ~BitmapSkin();
    private:
        HBITMAP hSkinBitmap = NULL;     // Handle to the bitmap
        BITMAP bm;

        void reportError(string error);
};

#endif // BITMAPSKIN_H
