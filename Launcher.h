#pragma once

#include "Rocket.h" 

using namespace System;
using namespace System::Drawing;
using namespace System::Collections::Generic; 

public ref class Launcher 
{
public:
    // Поля класса 
    PointF Position;           // Координаты пусковой установки в игровом мире
    int Id;                    // Уникальный идентификатор установки для отладки и отрисовки
    float MinLaunchIntervalSec; // Минимальное время перезарядки в секундах
    float MaxLaunchIntervalSec; // Максимальное время перезарядки в секундах
    float TimeToNextLaunchSec; // Собственный таймер перезарядки для этой конкретной установки

    // Статический объект Random, общий для всех экземпляров класса Launcher
    // Это важно, чтобы избежать создания нескольких генераторов с одинаковым начальным значением,
    // что привело бы к одинаковым "случайным" последовательностям
    static Random^ SharedRandom = gcnew Random();

    //  Конструктор класса 
    // Вызывается при создании новой пусковой установки
    Launcher(PointF pos, int id, float minInterval, float maxInterval) 
    {
        // Инициализация полей значениями, переданными в конструктор
        Position = pos;
        Id = id;
        MinLaunchIntervalSec = minInterval;
        MaxLaunchIntervalSec = maxInterval;
        // Сразу же устанавливаем начальный таймер перезарядки
        ResetLaunchTimer();
    }

    // Метод для сброса и установки нового таймера перезарядки 
    void ResetLaunchTimer() 
    {
        // Генерируем случайное число с плавающей точкой от 0.0 до 1.0,
        // масштабируем его до диапазона (Max - Min) и прибавляем Min
        // В результате получаем случайное время перезарядки в заданном интервале
        TimeToNextLaunchSec = (float)SharedRandom->NextDouble() * (MaxLaunchIntervalSec - MinLaunchIntervalSec) + MinLaunchIntervalSec;
    }

    // Метод "Выстрел" 
    // Создает и возвращает новую ракету
    // Этот метод вызывается из главного игрового цикла
    Rocket^ Fire(PointF targetPos, float rocketSpeed) 
    {
        // Сразу после выстрела сбрасываем таймер для этой установки, чтобы она начала перезаряжаться
        ResetLaunchTimer();
        // Создаем новый объект ракеты (используя gcnew) и возвращаем на него дескриптор (^)
        return gcnew Rocket(Position, targetPos, rocketSpeed);
    }

    // Метод для обновления внутреннего таймера 
    // Вызывается на каждом кадре, чтобы отсчитывать время перезарядки
    void UpdateOwnTimer(float deltaTime) 
    {
        // Если таймер еще не истек, уменьшаем оставшееся время
        if (TimeToNextLaunchSec > 0) 
        {
            TimeToNextLaunchSec -= deltaTime;
        }
    }

    // Метод отрисовки установки 
    void Draw(Graphics^ g, PointF worldOriginToScreenOrigin) 
    {
        // Преобразуем мировые координаты в экранные
        float screenX = Position.X + worldOriginToScreenOrigin.X;
        float screenY = Position.Y + worldOriginToScreenOrigin.Y;
        // Рисуем установку как темно-серый квадрат
        g->FillRectangle(Brushes::DarkGray, screenX - 5.0f, screenY - 5.0f, 10.0f, 10.0f);
        // Рисуем ее идентификационный номер сверху для отладки
        g->DrawString(Id.ToString(), gcnew System::Drawing::Font("Arial", 8), Brushes::White, screenX - 4, screenY - 5);
    }
};
