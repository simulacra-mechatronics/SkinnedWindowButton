#include "BitmapSkin.h"

BitmapSkin::BitmapSkin(HINSTANCE hInstance, WORD wBitmapResourceName)
{
    if(loadFromResource(hInstance, wBitmapResourceName) == false){
        reportError("Unable to load bitmap resource");
    }
    else{
        ::GetObject(hSkinBitmap, sizeof(bm), &bm);
    }
}

bool BitmapSkin::loadFromResource(HINSTANCE hInstance, WORD wBitmapResourceName){
    hSkinBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(wBitmapResourceName));
    if(hSkinBitmap == NULL){
        return false;
    }
    else{
        return true;
    }
}

void BitmapSkin::reportError(string error){
    std::cerr << error << endl;
}

HBITMAP BitmapSkin::getBitmapHandle(){
    return hSkinBitmap;
}

LONG BitmapSkin::getBitmapWidth(){
    return bm.bmWidth;
}

LONG BitmapSkin::getBitmapHeight(){
    return bm.bmHeight;
}

BitmapSkin::~BitmapSkin()
{
    //dtor
}
