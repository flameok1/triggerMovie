// triggerMovie.cpp : 定義應用程式的進入點。
//

#include "pch.h"
#include "framework.h"
#include "triggerMovie.h"
#include "VideoPlayer.h"
#include "CWindow.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp> // 影像處理相關功能 (例如 resize)
#include <opencv2/highgui.hpp> // 顯示影像相關功能 (例如 imshow)
#include <opencv2/dnn.hpp>
#include <thread>
#include <atomic>
#define USE_CV_MODE 2

std::atomic<bool> isRunning(true);

// 這個程式碼模組所包含之函式的向前宣告:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int doCapture();
VideoPlayer* g_player = nullptr;
CWindow* g_pNewWindow = nullptr;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    g_pNewWindow = new CWindow(L"Video Wnd", 1280, 720, WndProc, WS_OVERLAPPEDWINDOW);

    g_pNewWindow->show(); //SW_MAXIMIZE

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TRIGGERMOVIE));

    MSG msg;

    std::thread camThread(doCapture);

    // 主訊息迴圈:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    isRunning = false;
    delete g_pNewWindow;
    g_pNewWindow = nullptr;

    // 等待執行緒結束
    if (camThread.joinable()) {
        camThread.join();
    }

    return (int) msg.wParam;
}

//
//  函式: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  用途: 處理主視窗的訊息。
//
//  WM_COMMAND  - 處理應用程式功能表
//  WM_PAINT    - 繪製主視窗
//  WM_DESTROY  - 張貼結束訊息然後傳回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        g_player = new VideoPlayer(hWnd);
        if (!g_player->Init())
        {
            isRunning = false;
            PostQuitMessage(0);
        }
        return 0;

    case WM_SYSKEYDOWN:
        if ((GetAsyncKeyState(VK_MENU) & 0x8000) && wParam == VK_RETURN) {
            g_pNewWindow->ToggleFullscreen();
        }
        break;

    case WM_DESTROY:
        delete g_player;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int doCapture()
{
    cv::VideoCapture cap(0);  // 開啟預設攝影機
    if (!cap.isOpened()) {
        std::cerr << "無法開啟攝影機" << std::endl;
        return -1;
    }

#if USE_CV_MODE == 1
    // 載入人臉偵測模型（使用 Haar Cascades）
    cv::CascadeClassifier face_cascade;
    if (!face_cascade.load("haarcascade_frontalface_alt_tree.xml")) {
        std::cerr << "無法載入人臉分類器" << std::endl;
        return -1;
    }

    cv::Mat frame;  // 宣告一個 Mat 物件，用來存放每一幀攝影機影像

    while (isRunning) {
        cap >> frame;  // 從攝影機抓取一幀影像，存入 frame
        if (frame.empty()) break;  // 若影像為空（可能攝影機中斷），就跳出迴圈

        std::vector<cv::Rect> faces;  // 用來儲存偵測到的人臉區域（矩形）

        cv::Mat gray;  // 儲存轉為灰階的影像
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        // 將 BGR 彩色影像轉換為灰階（人臉偵測通常在灰階影像上執行，較快）

        cv::equalizeHist(gray, gray);
        // 對灰階影像做直方圖等化，增強對比度，有助於提高人臉偵測準確率

        face_cascade.detectMultiScale(gray, faces);
        // 使用訓練好的 Haar 分類器在灰階影像中偵測人臉，結果儲存在 faces 向量中

        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
        }

        if (faces.size() > 0)
        {
            if (g_player != NULL)
            {
                if (g_player->getNowType() == 0)
                {
                    g_player->PlayNext(1);
                }
            }
        }
        else
        {
            if (g_player->getNowType() == 1)
            {
                g_player->setNextType(0);
            }
        }

        cv::imshow("Face Detection", frame);
        // 顯示處理後的影像（含人臉框）在名為 "Face Detection" 的視窗中

        if (cv::waitKey(10) == 27) break;
        // 等待鍵盤輸入 10 毫秒，若按下 ESC 鍵（ASCII碼 27）則中止迴圈
    }

#elif USE_CV_MODE == 2
    cv::dnn::Net net = cv::dnn::readNetFromCaffe("cvdata/deploy.prototxt", "cvdata/res10_300x300_ssd_iter_140000.caffemodel");

    if (net.empty()) {
        std::cerr << "無法載入人臉分類器" << std::endl;
        return -1;
    }

    cv::Mat frame;  // 宣告一個 Mat 物件，用來存放每一幀攝影機影像
    bool isFind = false;

    while (isRunning) {
        cap >> frame;  // 從攝影機抓取一幀影像，存入 frame
        if (frame.empty()) break;  // 若影像為空（可能攝影機中斷），就跳出迴圈

        cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300, 300), cv::Scalar(104.0, 177.0, 123.0));

        net.setInput(blob);
        cv::Mat detections = net.forward();

        cv::Mat detectionMat(detections.size[2], detections.size[3], CV_32F, detections.ptr<float>());

        isFind = false;
        for (int i = 0; i < detectionMat.rows; ++i) {
            float confidence = detectionMat.at<float>(i, 2);
            if (confidence > 0.5) {
                int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
                int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
                int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
                int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

                cv::rectangle(frame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2);
                cv::putText(frame, cv::format("%.2f", confidence), cv::Point(x1, y1 - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);

                isFind = true;
            }
        }

        cv::imshow("Face Detection", frame);
        
        if (isFind)
        {
            if (g_player != NULL)
            {
                if (g_player->getNowType() == 0)
                {
                    g_player->PlayNext(1);
                }
            }
        }
        else
        {
            if (g_player->getNowType() == 1)
            {
                g_player->setNextType(0);
            }
        }

        if (cv::waitKey(10) == 27) break;
        // 等待鍵盤輸入 10 毫秒，若按下 ESC 鍵（ASCII碼 27）則中止迴圈
    }
#endif
    cap.release();
    cv::destroyAllWindows();
}