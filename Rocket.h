#pragma once

#include <cmath> 

; using namespace System::Drawing;
using namespace System;

public ref class Rocket 
{
public:
    // Поля класса 
    PointF Position;    // Текущие координаты ракеты в игровом мире
    PointF Velocity;    // Вектор скорости (направление и величина движения в секунду)
    float Speed;        // Скалярная скорость ракеты (длина вектора скорости)
    bool IsActive;      // Флаг, показывающий, активна ли ракета (летит ли она)
    bool IsIntercepted; // Флаг для отслеживания, была ли ракета перехвачена радаром

    // Конструктор класса 
    // Вызывается при создании нового объекта ракеты (gcnew Rocket())
    Rocket(PointF startPos, PointF targetPos, float speed)
    {
        // Инициализация начальных значений
        Position = startPos;
        Speed = speed;
        IsActive = true;
        IsIntercepted = false;

        // Расчет вектора скорости 
        // Находим разницу координат между целью и стартом, чтобы получить вектор направления
        float dx = targetPos.X - startPos.X;
        float dy = targetPos.Y - startPos.Y;
        // Вычисляем длину этого вектора направления (по теореме Пифагора)
        float length = (float)Math::Sqrt(dx * dx + dy * dy);

        // Нормализуем вектор (делаем его единичной длины) и умножаем на скалярную скорость
        // Это гарантирует, что ракета будет двигаться с заданной скоростью независимо от расстояния до цели
        if (length > 0) 
        {
            Velocity = PointF(dx / length * Speed, dy / length * Speed);
        }
        else 
        {
            // Если стартовая точка и цель совпадают, скорость будет нулевой, чтобы избежать деления на ноль
            Velocity = PointF(0, 0);
        }
    }

    // Метод обновления состояния ракеты 
    // Вызывается на каждом кадре игры для перемещения ракеты
    void Update(float deltaTime) 
    {
        // Если ракета неактивна (сбита или достигла цели), ничего не делаем
        if (!IsActive) return;
        // Обновляем позицию ракеты, добавляя смещение, основанное на векторе скорости и времени кадра
        Position.X += Velocity.X * deltaTime;
        Position.Y += Velocity.Y * deltaTime;
    }

    // Метод отрисовки ракеты 
    // Вызывается для отображения ракеты на экране
    void Draw(Graphics^ g, PointF worldOriginToScreenOrigin) 
    {
        // Не рисуем неактивные ракеты, чтобы они исчезали с экрана
        if (!IsActive) return;
        // Преобразуем игровые (мировые) координаты в экранные, добавляя смещение
        float screenX = Position.X + worldOriginToScreenOrigin.X;
        float screenY = Position.Y + worldOriginToScreenOrigin.Y;
        // Выбираем цвет кисти в зависимости от того, была ли ракета перехвачена
        Brush^ brush = IsIntercepted ? Brushes::LightGreen : Brushes::Red;
        // Рисуем ракету как небольшой закрашенный эллипс (кружок)
        g->FillEllipse(brush, screenX - 3.0f, screenY - 3.0f, 6.0f, 6.0f);
    }

    // Метод для вычисления расстояния до другой точки 
    // Удобная функция для проверки столкновений или дальности
    float GetDistanceTo(PointF p) 
    {
        // Находим катеты прямоугольного треугольника между двумя точками
        float dx = Position.X - p.X;
        float dy = Position.Y - p.Y;
        // Возвращаем гипотенузу (расстояние) по теореме Пифагора
        return (float)Math::Sqrt(dx * dx + dy * dy);
    }
};
