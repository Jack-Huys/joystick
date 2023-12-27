#include <iostream>
#include <Windows.h>
#include <dinput.h>
#include <cmath>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <QDebug>

// 定义阈值
const int axisThreshold = 6000; // XY轴阈值
const int rotationThreshold = 6000; // 旋转阈值

const float movementScale = 0.3f; // 移动级别
const float rotationScale = 0.3f; // 旋转级别


// 定义摇杆轴的最小值和最大值
const int MIN_AXIS_VALUE = 0;
const int MAX_AXIS_VALUE = 65535;

// 校准摇杆轴的数值
float CalibrateAxisValue(int rawValue)
{
    // 进行零点校准和范围映射
    float calibratedValue = (float)(rawValue - MIN_AXIS_VALUE) / (MAX_AXIS_VALUE - MIN_AXIS_VALUE) * 2.0f - 1.0f;

    return calibratedValue;
}


// 移动平均队列大小
#define SMOOTHING_QUEUE_SIZE 5

// 结构体定义
struct SmoothedAxis {
    int values[SMOOTHING_QUEUE_SIZE];
    int currentIndex;
};

// 初始化平滑处理结构体
void InitSmoothing(SmoothedAxis& axis) {
    for (int i = 0; i < SMOOTHING_QUEUE_SIZE; ++i) {
        axis.values[i] = 0;
    }
    axis.currentIndex = 0;
}

// 平滑处理函数
int SmoothAxisValue(SmoothedAxis& axis, int newValue) {
    axis.values[axis.currentIndex] = newValue;
    axis.currentIndex = (axis.currentIndex + 1) % SMOOTHING_QUEUE_SIZE;

    int smoothedValue = 0;
    for (int i = 0; i < SMOOTHING_QUEUE_SIZE; ++i) {
        smoothedValue += axis.values[i];
    }
    return smoothedValue / SMOOTHING_QUEUE_SIZE;
}

// 应用死区和平滑处理
//获取摇杆状态后，对摇杆的值进行处理，将较小的摇杆值设为0
//将连续的摇杆状态值进行平滑处理，例如使用移动平均值
void ApplyDeadzoneAndSmoothing(DIJOYSTATE2& joystickState, int deadzone, SmoothedAxis& smoothedX, SmoothedAxis& smoothedY, SmoothedAxis& smoothedZ) {
    // X轴死区设置
    if (joystickState.lX < deadzone && joystickState.lX > -deadzone) {
        joystickState.lX = 0;
    }

    // Y轴死区设置
    if (joystickState.lY < deadzone && joystickState.lY > -deadzone) {
        joystickState.lY = 0;
    }

    // Z轴死区设置
    if (joystickState.lZ < deadzone && joystickState.lZ > -deadzone) {
        joystickState.lZ = 0;
    }

    // 进行平滑处理
    joystickState.lX = SmoothAxisValue(smoothedX, joystickState.lX);
    joystickState.lY = SmoothAxisValue(smoothedY, joystickState.lY);
    joystickState.lZ = SmoothAxisValue(smoothedZ, joystickState.lZ);
}


// 处理摇杆按键信号
void ProcessJoystickInput(const DIJOYSTATE2& joystickState, const DIJOYSTATE2& prevJoystickState)
{
    //处理按键按下
    if (joystickState.rgbButtons[0] & 0x80 && !(prevJoystickState.rgbButtons[0] & 0x80))
    {
        qDebug() << "跟踪模式" ;
    }
    //else if (!(joystickState.rgbButtons[0] & 0x80) && prevJoystickState.rgbButtons[0] & 0x80)
    //{
    //    std::cout << "跟踪模式释放" << std::endl;
    //}
    if (joystickState.rgbButtons[1] & 0x80 && !(prevJoystickState.rgbButtons[1] & 0x80))
    {
        qDebug() << "手动锁向模式";
    }
    if (joystickState.rgbButtons[2] & 0x80 && !(prevJoystickState.rgbButtons[2] & 0x80))
    {
        qDebug() << "手动锁点" ;
    }
    if (joystickState.rgbButtons[3] & 0x80 && !(prevJoystickState.rgbButtons[3] & 0x80))
    {
        qDebug() <<"机构up";
    }
    if (joystickState.rgbButtons[4] & 0x80 && !(prevJoystickState.rgbButtons[4] & 0x80))
    {
        qDebug() << "机构down";
    }
    if (joystickState.rgbButtons[5] & 0x80 && !(prevJoystickState.rgbButtons[5] & 0x80))
    {
         qDebug() << "Infrared visible switching";
    }
    if (joystickState.rgbButtons[6] & 0x80 && !(prevJoystickState.rgbButtons[6] & 0x80))
    {
       qDebug() << "凝视" ;
    }
    if (joystickState.rgbButtons[7] & 0x80 && !(prevJoystickState.rgbButtons[7] & 0x80))
    {
        qDebug() << "回中" ;
    }
    /*
    if (joystickState.lX != prevJoystickState.lX || joystickState.lY != prevJoystickState.lY || joystickState.lZ != prevJoystickState.lZ)
    {
        // 处理左右移动
        if (joystickState.lX < -5000) {
            std::cout << "向左移动，移动值：" << joystickState.lX << std::endl;
        }
        else if (joystickState.lX > 5000) {
            std::cout << "向右移动，移动值：" << joystickState.lX << std::endl;
        }

        // 处理上下移动
        if (joystickState.lY < -5000) {
            std::cout << "向上移动，移动值：" << joystickState.lY << std::endl;
        }
        else if (joystickState.lY > 5000) {
            std::cout << "向下移动，移动值：" << joystickState.lY << std::endl;
        }

        // 处理旋转操作
        if (joystickState.lZ < -5000) {
            std::cout << "顺时针，旋转值：" << joystickState.lZ << std::endl;
        }
        else if (joystickState.lZ > 5000) {
            std::cout << "逆时针，旋转值：" << joystickState.lZ << std::endl;
        }
    }
    */

}

int main()
{
    // 初始化DirectInput
    LPDIRECTINPUT8       g_pDI;

    SmoothedAxis smoothedX, smoothedY, smoothedZ;
    InitSmoothing(smoothedX);
    InitSmoothing(smoothedY);
    InitSmoothing(smoothedZ);

    HRESULT hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDI, NULL);

    if (FAILED(hr))
    {
        // 初始化失败
        std::cerr << "DirectInput8Create failed." << std::endl;
        return 1;
    }
    else
    {
        // 初始化成功，可以使用g_pDI指向的DirectInput对象进行操作
        // 创建手柄设备
        LPDIRECTINPUTDEVICE8 g_pJoystickDevice;
        hr = g_pDI->CreateDevice(GUID_Joystick, &g_pJoystickDevice, nullptr);
        if (FAILED(hr))
        {
            // 创建失败
            std::cerr << "CreateDevice failed." << std::endl;
            g_pDI->Release();
            g_pDI = nullptr;
            return 1;
        }
        else
        {
            // 设置数据格式
            hr = g_pJoystickDevice->SetDataFormat(&c_dfDIJoystick2);
            if (FAILED(hr))
            {
                // 设置数据格式失败
                std::cerr << "SetDataFormat failed" << std::endl;
                g_pDI->Release();
                g_pDI = nullptr;
                return 1;
            }

            // 设置协作级别
            //HWND hwnd = GetDesktopWindow();  // 假设使用桌面窗口句柄
            //后台
            hr = g_pJoystickDevice->SetCooperativeLevel(nullptr, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
            if (FAILED(hr))
            {
                // 设置协作级别失败
                std::cerr << "SetCooperativeLevel failed" << std::endl;
                g_pDI->Release();
                g_pDI = nullptr;
                return 1;
            }

            // Acquire手柄
            hr = g_pJoystickDevice->Acquire();
            if (FAILED(hr))
            {
                // Acquire失败
                std::cerr << "Acquire failed" << std::endl;
                g_pDI->Release();
                g_pDI = nullptr;
                return 1;
            }

            DIJOYSTATE2 prevJoystickState;
            ZeroMemory(&prevJoystickState, sizeof(DIJOYSTATE2));

            while (true)
            {
                DIJOYSTATE2 joystickState;
                hr = g_pJoystickDevice->Poll();
                if (SUCCEEDED(hr))
                {
                    hr = g_pJoystickDevice->GetDeviceState(sizeof(DIJOYSTATE2), &joystickState);
                    if (SUCCEEDED(hr))
                    {

                        // 对摇杆状态进行死区设置和平滑处理
                        //ApplyDeadzoneAndSmoothing(joystickState, 2000, smoothedX, smoothedY, smoothedZ);

                        ProcessJoystickInput(joystickState, prevJoystickState);

                        // 获取X轴、Y轴和Z轴的值
                        int xAxis = joystickState.lX;
                        int yAxis = joystickState.lY;
                        int zAxis = joystickState.lZ;

                        // 对数据进行校准
                        float calibratedX = CalibrateAxisValue(xAxis);
                        float calibratedY = CalibrateAxisValue(yAxis);
                        float calibratedYZ= CalibrateAxisValue(zAxis);
                         // 计算轴值的变化
                        //int xAxisChange = joystickState.lX - prevJoystickState.lX;
                        //int yAxisChange = joystickState.lY - prevJoystickState.lY;
                        //int zAxisChange = joystickState.lZ - prevJoystickState.lZ;
                        // 在控制台中显示轴的值
                        std::cout << "X Axis: " << calibratedX << "  Y Axis: " << calibratedY << "  Z Axis: " << calibratedYZ << std::endl;
                        //更新上一次摇杆状态
                        prevJoystickState = joystickState;
                    }
                    else if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
                    {
                        hr = g_pJoystickDevice->Acquire();
                    }
                    else
                    {
                        std::cerr << "GetDeviceState error" << std::endl;
                    }
                }
                else
                {
                    hr = g_pJoystickDevice->Acquire();
                }

                Sleep(100);
            }

            // 释放手柄设备
            if (g_pJoystickDevice)
            {
                g_pJoystickDevice->Unacquire();
                g_pJoystickDevice->Release();
                g_pJoystickDevice = nullptr;
            }
        }
        // 在程序结束时记得释放DirectInput对象
        if (g_pDI)
        {
            g_pDI->Release();
            g_pDI = nullptr;
        }
    }
    return 0;
}
