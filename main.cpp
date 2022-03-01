
#include <Windows.h>
#include <opencv2/opencv.hpp>

#define SUCCESS 1

using namespace cv;

    BITMAPINFOHEADER createBitmapHeader(int width, int height)
    {
        BITMAPINFOHEADER  bi;

        // create a bitmap
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = width;
        bi.biHeight = -height;  //this is the line that makes it draw upside down or not
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = 0;
        bi.biYPelsPerMeter = 0;
        bi.biClrUsed = 0;
        bi.biClrImportant = 0;

        return bi;
    }

    Mat captureScreenMat(HWND hwnd)
    {
        Mat src;

        // get handles to a device context (DC)
        HDC hwindowDC = GetDC(hwnd);
        HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
        SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

        // define scale, height and width
        int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
        int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

        // create mat object
        src.create(height, width, CV_8UC4);

        // create a bitmap
        HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
        BITMAPINFOHEADER bi = createBitmapHeader(width, height);

        // use the previously created device context with the bitmap
        SelectObject(hwindowCompatibleDC, hbwindow);

        // copy from the window device context to the bitmap device context
        StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);  //change SRCCOPY to NOTSRCCOPY for wacky colors !
        GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);            //copy from hwindowCompatibleDC to hbwindow

        // avoid memory leak
        DeleteObject(hbwindow);
        DeleteDC(hwindowCompatibleDC);
        ReleaseDC(hwnd, hwindowDC);

        return src;
    }

    Mat transformImage(Mat& src, Mat& debug)
    {
        Mat				imgHSV, imgCrop, mask;
        int				hmin = 0, smin = 0, vmin = 153, hmax = 54, smax = 176, vmax = 255;
        Rect			roi(550, 200, 300, 300);
        Scalar			lower(hmin, smin, vmin);
        Scalar			upper(hmax, smax, vmax);
        
        // Crop the image
        imgCrop = src(roi);

        // Change color the image
        cvtColor(imgCrop, imgHSV, COLOR_BGR2HSV);

        // Make the alert white and ohter stuff disappear
        inRange(imgHSV, lower, upper, mask);
        //debug = imgHSV;
        return mask;
    }

    bool    detectAlert(Mat& src)
    {
        //int total = src.rows * src.cols;

        //std::cout << src.cols << " x " << src.rows << std::endl;
        //std::cout << "total: " << total << std::endl;
        //std::cout << "non_zero: " << countNonZero(src) << std::endl;
        if (countNonZero(src) >= 80)
            return true;
        else
            return false;
    }

    void    pressKeyE(void)
    {
        // input event.
        INPUT ip;

        // Set up a generic keyboard event.
        ip.type = INPUT_KEYBOARD;
        ip.ki.wScan = 0; // hardware scan code for key
        ip.ki.time = 0;
        ip.ki.dwExtraInfo = 0;

        // Press the "A" key
        ip.ki.wVk = 0x45; // virtual-key code for the "a" key
        ip.ki.dwFlags = 0; // 0 for key press
        SendInput(1, &ip, sizeof(INPUT));

        // Release the "A" key
        ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
        SendInput(1, &ip, sizeof(INPUT));
    }

    int analyzeImages()
    {
        int i = 0;
        while (true)
        {
            // break the loop in case I dont find the image i want
            if (i > 50)
                return -1;

            // capture image
            HWND hwnd = GetDesktopWindow();
            Mat src = captureScreenMat(hwnd);
            Mat debug;
            //Mat test = imread("fishing2.png");

            // transform image and check for eclamation mark
            Mat transformSrc = transformImage(src, debug);
            if (detectAlert(transformSrc) == true)
            {
                //imwrite("src.png", src);
                //imwrite("debug.png", debug);
                //imwrite("BnW.png", transformSrc);
                i++;
                return 0;
            }

            // display img
            //imshow("Screenshot.png", transformSrc);
            //waitKey(1);
        }
    }

    void    routine(int &i)
    {
        pressKeyE();
        Sleep(3000);
        if (analyzeImages() == SUCCESS)
        {
            pressKeyE();
            Sleep(3000);
            i++;
        }
    }

    int main()
    {
        // Nb of success
        int i = 0;

        //The time you need to adjust cursor in the water
        Sleep(5000);

        while (i < 200)
            routine(i);
        return 0;
    }
