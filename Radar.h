#pragma once // Предотвращает повторное включение этого файла

#include "Rocket.h"
#include <cmath>

// Определяем константу M_PI, если она еще не определена
// Это обеспечивает совместимость, так как не все компиляторы определяют ее по умолчанию
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace System;
using namespace System::Drawing;            
using namespace System::Drawing::Drawing2D;  


public ref class Radar 
{
public:
    // Поля класса 

    PointF Position;                  // Координаты центра радара в мировом пространстве
    float CurrentAngleDegrees;        // Текущий угол поворота луча радара в градусах
    float RotationSpeedDps;           // Скорость вращения радара в градусах в секунду
    float BeamWidthDegrees;           // Ширина основного луча радара в градусах
    float MaxDetectionRangeP;         // Максимальная дальность пассивного обнаружения (внешний синий круг)
    float BeamEffectiveRadiusR;       // Эффективная дальность активного луча перехвата (голубой сектор)
    float CircularAttackRange;        // Радиус круговой зоны атаки (оранжевый круг)
    float CoreVulnerabilityRadius;    // Радиус уязвимого ядра радара (не используется в коде, но может быть нужен для логики попаданий)
    float DeadZoneRadius;             // Радиус "мертвой зоны" вокруг радара, где цели не обнаруживаются
    bool IsDestroyed;                 // Флаг, указывающий, уничтожен ли радар

    // Конструктор класса Radar
    // Инициализирует объект радара с заданными параметрами
    Radar(PointF pos, float rotSpeed, float beamWidth, float p_range, float r_beam_eff_radius,
        float circularAttackRng, float coreVulnerabilityR, float deadZoneRad)
    {
        Position = pos;
        RotationSpeedDps = rotSpeed;
        BeamWidthDegrees = beamWidth;
        MaxDetectionRangeP = p_range;
        BeamEffectiveRadiusR = r_beam_eff_radius;
        CircularAttackRange = circularAttackRng;
        CoreVulnerabilityRadius = coreVulnerabilityR;
        DeadZoneRadius = deadZoneRad;
        // Начальные значения при создании
        CurrentAngleDegrees = 0.0f;
        IsDestroyed = false;
    }

    // Метод для обновления состояния радара, вызывается на каждом кадре
    // deltaTime - время, прошедшее с момента последнего обновления
    void Update(float deltaTime) 
    {
        if (IsDestroyed) return; // Если радар уничтожен, ничего не делаем
        // Поворачиваем радар в соответствии с его скоростью и прошедшим временем
        CurrentAngleDegrees += RotationSpeedDps * deltaTime;
        // Нормализуем угол, чтобы он оставался в диапазоне [0, 360)
        CurrentAngleDegrees = NormalizeAngle(CurrentAngleDegrees);
    }

    // Вспомогательная функция для приведения угла к диапазону [0, 360)
    float NormalizeAngle(float angle) 
    {
        // fmod - остаток от деления для чисел с плавающей точкой
        angle = fmod(angle, 360.0f);
        // Если угол отрицательный, добавляем 360, чтобы сделать его положительным
        if (angle < 0) angle += 360.0f;
        return angle;
    }

    // Вспомогательная функция для вычисления кратчайшей разницы между двумя углами
    // Возвращает значение в диапазоне [-180, 180]
    float AngleDifference(float angle1, float angle2) 
    {
        float diff = NormalizeAngle(angle1) - NormalizeAngle(angle2);
        // Корректируем разницу, чтобы найти кратчайший путь на окружности
        if (diff > 180.0f) diff -= 360.0f;
        if (diff < -180.0f) diff += 360.0f;
        return diff;
    }

    // Основной метод логики: обнаружение и перехват ракеты
    // Возвращает true, если ракета была перехвачена, иначе false
    bool DetectAndIntercept(Rocket^ rocket) 
    {
        // Базовые проверки: не работаем, если радар уничтожен или ракета неактивна
        if (IsDestroyed || !rocket->IsActive) return false;

        // Вычисляем расстояние от центра радара до ракеты
        float distToRocket = rocket->GetDistanceTo(Position);

        // 1. Проверка на мертвую зону
        // Если ракета слишком близко, радар ее игнорирует
        if (distToRocket <= DeadZoneRadius) {
            return false; // Ракета в мертвой зоне, перехват невозможен
        }

        // 2. Проверка на попадание в основной луч перехвата (голубой сектор)
        // Сначала проверяем, находится ли ракета в пределах эффективной дальности луча
        if (distToRocket <= BeamEffectiveRadiusR) 
        {
            // Рассчитываем угол от радара к ракете
            float angleToRocketRad = Math::Atan2(rocket->Position.Y - Position.Y, rocket->Position.X - Position.X);
            // Конвертируем радианы в градусы
            float angleToRocketDeg = angleToRocketRad * 180.0f / (float)M_PI;
            angleToRocketDeg = NormalizeAngle(angleToRocketDeg); // Приводим угол к [0, 360)

            // Находим разницу между углом радара и углом к ракете
            float angleDiff = AngleDifference(angleToRocketDeg, CurrentAngleDegrees);

            // Проверяем, находится ли ракета внутри углового сектора луча
            if (Math::Abs(angleDiff) <= BeamWidthDegrees / 2.0f) 
            {
                rocket->IsIntercepted = true; // Помечаем ракету как перехваченную.
                rocket->IsActive = false;     // Уничтожаем ракету
                return true;                  
            }
        }
        return false; // Ракета не была перехвачена
    }

    // Метод для отрисовки радара и его компонентов на экране
    // g - объект Graphics, на котором происходит рисование
    // worldOriginToScreenOrigin - смещение для преобразования мировых координат в экранные
    void Draw(Graphics^ g, PointF worldOriginToScreenOrigin) 
    {
        // Рассчитываем экранные координаты центра радара
        float screenX = Position.X + worldOriginToScreenOrigin.X;
        float screenY = Position.Y + worldOriginToScreenOrigin.Y;

        // 1. Отрисовка зоны максимального обнаружения (P)
        if (!IsDestroyed) 
        {
            Pen^ detectionZonePen = gcnew Pen(Color::FromArgb(60, Color::CornflowerBlue), 1.5f);
            detectionZonePen->DashStyle = DashStyle::Dot; // Пунктирная линия (точки).
            g->DrawEllipse(detectionZonePen, screenX - MaxDetectionRangeP, screenY - MaxDetectionRangeP, MaxDetectionRangeP * 2, MaxDetectionRangeP * 2);
        }

        // 2. Отрисовка базы радара
        Brush^ baseBrush = IsDestroyed ? Brushes::DarkRed : Brushes::Blue; // Цвет зависит от состояния
        g->FillEllipse(baseBrush, screenX - 10, screenY - 10, 20.0f, 20.0f);

        // 3. Отрисовка зоны круговой атаки (оранжево-красная)
        Pen^ circularAttackPen = gcnew Pen(Color::OrangeRed, 2);
        circularAttackPen->DashStyle = DashStyle::Dash; // Пунктирная линия (тире)
        g->DrawEllipse(circularAttackPen, screenX - CircularAttackRange, screenY - CircularAttackRange, CircularAttackRange * 2, CircularAttackRange * 2);

        // 4. Отрисовка мертвой зоны радара
        if (!IsDestroyed) 
        {
            Pen^ deadZonePen = gcnew Pen(Color::FromArgb(100, Color::DimGray), 1.5f);
            deadZonePen->DashStyle = DashStyle::DashDot; // Пунктирная линия (штрих-точка)
            g->DrawEllipse(deadZonePen, screenX - DeadZoneRadius, screenY - DeadZoneRadius, DeadZoneRadius * 2, DeadZoneRadius * 2);
        }

        // Если радар уничтожен, дальнейшую отрисовку (луч) не производим
        if (IsDestroyed) return;

        // 5. Отрисовка основного луча радара (голубой сектор)
        // Конвертируем углы в радианы для тригонометрических функций
        float angleRad = CurrentAngleDegrees * (float)M_PI / 180.0f;
        float beamHalfWidthRad = (BeamWidthDegrees / 2.0f) * (float)M_PI / 180.0f;

        // Определяем три точки, формирующие сектор 
        PointF p1 = PointF(screenX, screenY);
        PointF p2 = PointF(screenX + (float)(BeamEffectiveRadiusR * Math::Cos(angleRad - beamHalfWidthRad)), screenY + (float)(BeamEffectiveRadiusR * Math::Sin(angleRad - beamHalfWidthRad)));
        PointF p3 = PointF(screenX + (float)(BeamEffectiveRadiusR * Math::Cos(angleRad + beamHalfWidthRad)), screenY + (float)(BeamEffectiveRadiusR * Math::Sin(angleRad + beamHalfWidthRad)));

        // Создаем массив точек для полигона
        array<PointF>^ beamPoints = { p1, p2, p3 };
        // Заливаем полигон полупрозрачным цветом
        g->FillPolygon(gcnew SolidBrush(Color::FromArgb(100, Color::LightSkyBlue)), beamPoints);
        // Рисуем границы сектора
        g->DrawLine(Pens::SkyBlue, p1, p2);
        g->DrawLine(Pens::SkyBlue, p1, p3);
    }
};
