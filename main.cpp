#include <windows.h>
#include "CustomDialogButton.h"
#include "CustomSkinnedButton.h"
#include "BitmapSkin.h"
#include <iostream>     // Used for outputting info via 'cout' or 'cerr' to debug window
#include "resource.h"   // Contains declarations of resources contained in resource file

typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);   // Get a function pointer
lpfnSetLayeredWindowAttributes SetLayeredWindowAttributes;  // Set instance of function pointer

void DestroyCaption(HWND hwnd, int windowWidth, int windowHeight);  // Used to remove window caption and size window to bitmap size
BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ExitButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RunButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static COLORREF colorKey   = RGB(255,0,255);        // Color used in bitmap to designate transparent areas
static DWORD LWA_COLORKEY  = 0x00000001;            // Use colorKey as the transparency color
BOOL destroyCaption = false;                        // A boolean flag to determine whether the dialog window caption has been removed.

BitmapSkin* pDialogSkin;
CustomSkinnedButton* pRunButton;
CustomSkinnedButton* pExitButton;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // Load our bitmaps
    pDialogSkin = new BitmapSkin(hInstance, IDB_LauncherBackground);
    pRunButton = new CustomSkinnedButton(hInstance, IDB_RunOut, IDB_RunOver, IDB_RunIn, colorKey);
    pExitButton = new CustomSkinnedButton(hInstance, IDB_ExitOut, IDB_ExitOver, IDB_ExitIn, colorKey);

    // Import function to make windows transparent
    HMODULE hUser32 = GetModuleHandle(("USER32.DLL"));
    SetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");
    if(SetLayeredWindowAttributes == NULL)
        std::cerr << "Error: could not load window transparency. Could not load User32.DLL" << std::endl;

    // Create the window class for our 'Run' button
    WNDCLASS runWndclass;
    runWndclass.style = CS_HREDRAW | CS_VREDRAW;
    runWndclass.lpfnWndProc = RunButtonWndProc;         // Where we specify the name of the window procedure
    runWndclass.cbClsExtra = 0;
    runWndclass.cbWndExtra = 0;
    runWndclass.hInstance = hInstance;
    runWndclass.hIcon = NULL;
    runWndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
    runWndclass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    runWndclass.lpszMenuName = NULL;
    runWndclass.lpszClassName = TEXT ("runBtnProc");    // This value is reference by the IDRUN control in the RC file
    RegisterClass (&runWndclass);

    // Create the window class for our 'Exit' button
    WNDCLASS exitWndclass;
    exitWndclass.style = CS_HREDRAW | CS_VREDRAW;
    exitWndclass.lpfnWndProc = ExitButtonWndProc;       // Where we specify the name of the window procedure
    exitWndclass.cbClsExtra = 0;
    exitWndclass.cbWndExtra = 0;
    exitWndclass.hInstance = hInstance;
    exitWndclass.hIcon = NULL;
    exitWndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
    exitWndclass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
    exitWndclass.lpszMenuName = NULL;
    exitWndclass.lpszClassName = TEXT ("exitBtnProc");  // This value is referenced by the IDEXIT control in the RC file
    RegisterClass (&exitWndclass);

    // Launch the dialog window
    return DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);

    delete pDialogSkin;
    delete pRunButton;
    delete pExitButton;
}

// Window procedure for the owner-draw custom control IDEXIT ('Exit' button)
LRESULT CALLBACK ExitButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc ;                                // Handle to the button's device context

    switch (message)                                // Test for specific messages
    {

	case WM_LBUTTONDBLCLK:                          // Left mouse button has been clicked
        PostMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);  // Pass message back into this window procedure as a WM_LBUTTONDOWN message (Left mouse button down)
        break;

	case WM_MOUSEMOVE:                              // Mouse has been detected moving (hovering) over the button
	{
        if(pExitButton->isMouseOver() == false){    // If the hover flag has not been set
            pExitButton->mouseIsOver();             // Set the hover flag to true
            pRunButton->mouseIsOut();               // Make sure the hover flag for the other flag is turned off
            InvalidateRgn(hwnd, NULL, FALSE);       // Cause this button to be redrawn
            InvalidateRgn(GetDlgItem(GetParent(hwnd),IDRUN), NULL, FALSE);  // Redraw the other button just in case it was also recently flagged as in hover
        }

        return (0);
	}
	break;

    case WM_CREATE:                                 // Called when the button is first created
        SetWindowPos(hwnd, NULL, 0,0,pExitButton->getBitmapWidth(), pExitButton->getBitmapHeight(), SWP_NOMOVE | SWP_NOZORDER);   // Set the button to the width and height of the bitmaps being used
        SetWindowRgn(hwnd, pExitButton->getHandleToVisibleRegion(), true);     // Set the region as the visible area
        DeleteObject(pExitButton->getHandleToVisibleRegion());                 // Delete the region
    break;

    case WM_PAINT:                                  // Draw the visible region of the button
        HDC dcSkin;                                 // Handle to a compatible memory device context used for drawing the bitmap to the visible area
        BITMAP bm;                                  // Bitmap structure
        PAINTSTRUCT ps;                             // Paint structure
        hdc = BeginPaint(hwnd, &ps);                // Set the handle to the device context to the drawable area
        dcSkin = CreateCompatibleDC(hdc);           // Create a memory device context that exits only in memory

        if(pExitButton->getbuttonState() == BUTTONSTATE_IN){ // Check to see if the mouse is hovering over the button AND that the left mouse button is held down
            GetObject(pExitButton->getHandleButtonInBitmap(), sizeof(bm), &bm); // Store information about the bitmap used to represent this state in the bitmap structure
            SelectObject(dcSkin, pExitButton->getHandleButtonInBitmap()); // Select this bitmap into the memory device context
        }
        else if(pExitButton->getbuttonState() == BUTTONSTATE_OVER){            // Check to see if the mouse is only hovering
            GetObject(pExitButton->getHandleButtonOverBitmap(), sizeof(bm), &bm);
            SelectObject(dcSkin, pExitButton->getHandleButtonOverBitmap());
        }
        else{                                       // The mouse is neither hovering nor is the left button being held down
            GetObject(pExitButton->getHandleButtonOutBitmap(), sizeof(bm), &bm);
            SelectObject(dcSkin, pExitButton->getHandleButtonOutBitmap());
        }

        BitBlt(hdc, 0,0,pExitButton->getBitmapWidth(),pExitButton->getBitmapHeight(), dcSkin, 0, 0, SRCCOPY);  // Performs bit-block transfer of bitmap pixels to the memory device context
        DeleteDC(dcSkin);                           // Delete the memory device context
        EndPaint(hwnd, &ps);                        // Exits the painting process for the 'Run' button
    return 0 ;

    case WM_LBUTTONUP:                              // Left mouse button has been released over the 'Run' Button
        if(pExitButton->isMouseLeftButtonDown())    // Make sure that the left mouse button was clicked while over the exit button
            SendMessage(GetParent(hwnd), WM_COMMAND, GetWindowLong(hwnd, GWL_ID), (LPARAM)hwnd);    // Send this message to the dialog's main window procedure for processing.
    return 0;

    case WM_LBUTTONDOWN:                            // The left mouse button is being held down
    {
        pExitButton->mouseLeftDown();               // Set a flag to signify that the left mouse button is being held down
        if(pRunButton->isMouseLeftButtonDown())     // If the left mouse button was previously set while over the 'Run' button but not reset before the mouse was moved over to the 'Exit' button
            pRunButton->mouseLeftUp();              // Set the flag signifying that the left mouse button is down over the 'Run' button to false
        InvalidateRgn(hwnd, NULL, FALSE);           // Redraw this button with the image signifying that the button is being held down
        break;
    }

    }

    return DefWindowProc (hwnd, message, wParam, lParam);   // Send any unhandled messages back to the main dialog window procedure
}


// Window procedure for the owner-draw custom control IDRUN ('Run' button) - See previous function for comments
LRESULT CALLBACK RunButtonWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc ;

    switch (message)
    {

	case WM_LBUTTONDBLCLK:
        PostMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
        break;

	case WM_MOUSEMOVE:
	{
        if(pRunButton->isMouseOver() == false){
            pRunButton->mouseIsOver();
            pExitButton->mouseIsOut();
            InvalidateRgn(hwnd, NULL, FALSE);
            InvalidateRgn(GetDlgItem(GetParent(hwnd),IDEXIT), NULL, FALSE);
        }

        return (0);
	}
	break;

    case WM_CREATE:
        SetWindowPos(hwnd, NULL, 0,0,pRunButton->getBitmapWidth(), pRunButton->getBitmapHeight(), SWP_NOMOVE | SWP_NOZORDER);
        SetWindowRgn(hwnd, pRunButton->getHandleToVisibleRegion(), true);
        DeleteObject(pRunButton->getHandleToVisibleRegion());
    break;

    case WM_PAINT :
        HDC dcSkin;
        BITMAP bm;
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        dcSkin = CreateCompatibleDC(hdc);

        if(pRunButton->getbuttonState() == BUTTONSTATE_IN){
            GetObject(pRunButton->getHandleButtonInBitmap(), sizeof(bm), &bm);
            SelectObject(dcSkin, pRunButton->getHandleButtonInBitmap());
        }
        else if(pRunButton->getbuttonState() == BUTTONSTATE_OVER){
            GetObject(pRunButton->getHandleButtonOverBitmap(), sizeof(bm), &bm);
            SelectObject(dcSkin, pRunButton->getHandleButtonOverBitmap());
        }
        else{
            GetObject(pRunButton->getHandleButtonOutBitmap(), sizeof(bm), &bm);
            SelectObject(dcSkin, pRunButton->getHandleButtonOutBitmap());
        }

        BitBlt(hdc, 0,0,pRunButton->getBitmapWidth(),pRunButton->getBitmapHeight(), dcSkin, 0, 0, SRCCOPY);
        DeleteDC(dcSkin);
        EndPaint(hwnd, &ps);
    return 0 ;

    case WM_LBUTTONUP:
        if(pRunButton->isMouseLeftButtonDown())
            SendMessage(GetParent(hwnd), WM_COMMAND, GetWindowLong(hwnd, GWL_ID), (LPARAM)hwnd);
    return 0;

    case WM_LBUTTONDOWN:
    {
        pRunButton->mouseLeftDown();
        if(pExitButton->isMouseLeftButtonDown())
            pExitButton->mouseLeftUp();
        InvalidateRgn(hwnd, NULL, FALSE);
        break;
    }

    }

    return DefWindowProc (hwnd, message, wParam, lParam);
}

// The main window procedure used by the dialog window
BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hRunButtonWnd;                      // A handle to the 'Run' button
    static HWND hExitButtonWnd;                     // A handle to the 'Exit' button

    switch(uMsg)                                    // Search through messages sent to this window procedure
    {
    case WM_INITDIALOG:                             // Creating of the dialog window
        if(SetLayeredWindowAttributes != NULL)      // Make sure that this function exits
        {
            if(destroyCaption == false) {           // Make sure that the caption has not already been destroyed
                DestroyCaption(hwndDlg, pDialogSkin->getBitmapWidth(), pDialogSkin->getBitmapHeight());   // Destroy any window caption that may be set
                destroyCaption = true;              // Set a flag to ensure that this has been accomplished
            }

          SetWindowLong(hwndDlg, GWL_EXSTYLE, GetWindowLong(hwndDlg, GWL_EXSTYLE) | WS_EX_LAYERED); // Set the window style
          SetLayeredWindowAttributes(hwndDlg, colorKey, 0, LWA_COLORKEY);                           // Set the transparency color key
        }
        hRunButtonWnd = ::GetDlgItem(hwndDlg,IDRUN);    // Get the window handle for the 'Run' button
        hExitButtonWnd = ::GetDlgItem(hwndDlg,IDEXIT);  // Get the window handle for the 'Exit' button
    return TRUE;

    // draw our bitmap
    case WM_PAINT:                                  // Draw the dialog window
        {
        BITMAP bm;                                  // Create a bitmap structure
        PAINTSTRUCT ps;                             // Create a paint structure
        HDC hdc = BeginPaint(hwndDlg, &ps);         // Create a device context used for the dialog window
        HDC dcSkin = CreateCompatibleDC(hdc);       // Create a compatible memory device context to copy the color information from the bitmap to
        GetObject(pDialogSkin->getBitmapHandle(), sizeof(bm), &bm);      // Fill bitmap structure with information about the background image bitmap
        SelectObject(dcSkin, pDialogSkin->getBitmapHandle());            // Select this bitmap into the memory device context
        BitBlt(hdc, 0,0,pDialogSkin->getBitmapWidth(),pDialogSkin->getBitmapHeight(), dcSkin, 0, 0, SRCCOPY);   // Performs bit-block transfer of bitmap pixels to the memory device context
        DeleteDC(dcSkin);                           // Release the memory device context
        EndPaint(hwndDlg, &ps);                     // End painting of dialog window
        }
    return true;

    case WM_CLOSE:                                  // Exit application
    {
        EndDialog(hwndDlg, 0);                      // Close down the dialog window
    }
    return TRUE;

    case WM_COMMAND:                                // Button has been clicked
    {
        switch (LOWORD (wParam))
        {
            case IDRUN:                             // 'Run' button was clicked
                EndDialog (hwndDlg, 0) ;            // Close the dialog window (or anything else you want to happen)
                return TRUE ;
            case IDEXIT :                           // 'Exit' button was clicked
                EndDialog (hwndDlg, 0) ;            // Close the dialog window because the user is exiting the application
                return TRUE ;
        }
    }
    return TRUE;

    case WM_MOUSEMOVE:                              // Mouse has been moved while over the dialog window area
    {
        if(pRunButton->isMouseOver()){              // Check to see if the mouse was previously over the 'Run' button
            pRunButton->mouseIsOut();               // Set a flag to signify that the mouse is not hovering over the 'Run' button any more

            if(pRunButton->isMouseLeftButtonDown()){// Check to see if the mouse button was previously flagged as down over the 'Run' button
                pRunButton->mouseLeftUp();          // Set a flag to signify that the 'Run' button does not have the left mouse button clicked over it any more

            }
            InvalidateRgn(hRunButtonWnd, NULL, FALSE);  // Redraw the 'Run' button with the default state
        }
        if(pExitButton->isMouseOver()){             // Check to see if the mouse was previously over the 'Exit' button
            pExitButton->mouseIsOut();                 // Set a flag to signify that the mouse is not hovering over the 'Exit' button any more
            if(pExitButton->isMouseLeftButtonDown()){// Check to see if the mouse button was previously flagged as down over the 'Exit' button
                pExitButton->mouseLeftUp();               // Set a flag to signify that the 'Exit' button does not have the left mouse button clicked over it any more

            }
            InvalidateRgn(hExitButtonWnd, NULL, FALSE); // Redraw the 'Exit' button with the default state
        }
    }
    break;

    // Moves the window when the user clicks anywhere not covered by a control. HTCAPTION specifies
    // that all button clicks originate in the title bar area - even when the window has no title bar.
    case WM_LBUTTONDOWN:
    {
        PostMessage(hwndDlg, WM_NCLBUTTONDOWN, HTCAPTION,0);
    }
    return TRUE;

    case WM_LBUTTONUP:                              // The left mouse button was released
    {
        if(pRunButton->isMouseLeftButtonDown()){    // Check to see if the mouse button was previously flagged as down over the 'Run' button
            pRunButton->mouseLeftUp();              // Set a flag to signify that the 'Run' button does not have the left mouse button clicked over it any more
            InvalidateRgn(hRunButtonWnd, NULL, FALSE);  // Redraw the 'Run' button in its default state
        }
        if(pExitButton->isMouseLeftButtonDown()){   // Check to see if the mouse button was previously flagged as down over the 'Exit' button
            pExitButton->mouseLeftUp();                  // Set a flag to signify that the 'Exit' button does not have the left mouse button clicked over it any more
            InvalidateRgn(hExitButtonWnd, NULL, FALSE); // Redraw the 'Exit' button in its default state
        }
    }

    }
    return FALSE;
}




// Destroy our windows caption
void DestroyCaption(HWND hwnd, int windowWidth, int windowHeight)
{
 DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
 dwStyle &= ~(WS_CAPTION|WS_SIZEBOX);

 SetWindowLong(hwnd, GWL_STYLE, dwStyle);
 InvalidateRect(hwnd, NULL, true);
 SetWindowPos(hwnd, NULL, 0,0,windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);
}
