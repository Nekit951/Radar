#pragma once // Предотвращает повторное включение этого файла

#include <string>   
#include <fstream>  
#include <msclr/marshal_cppstd.h>

// Добавляем пространство имен для использования Dictionary
using namespace System::Collections::Generic;
using namespace System;
using namespace System::IO;
using namespace System::Windows::Forms;

public ref struct ConfigData
{
    // ШАГ 1: Создаем enum для ключей конфигурации 
private:
    enum class ConfigKey {
        RocketSpeed,
        DistanceCornerToCenter,
        RadarBeamWidthDegrees,
        RadarRotationSpeedDps,
        RadarMaxDetectionRangeP,
        RadarCircularAttackRange,
        RadarCoreVulnerabilityRadius,
        RadarDeadZoneRadius,
        LaunchIntervalMinSec,
        LaunchIntervalMaxSec,
        TotalRocketsToLaunch,
        Unknown // Для неизвестных ключей
    };

    // ШАГ 2: Создаем статический словарь для сопоставления строк и enum 
    static Dictionary<String^, ConfigKey>^ keyMap;

public:
    // Поля для хранения параметров конфигурации 
    float RocketSpeed;
    float DistanceCornerToCenter;
    float RadarBeamWidthDegrees;
    float RadarRotationSpeedDps;
    float RadarMaxDetectionRangeP;
    float RadarBeamEffectiveRadiusR;
    float RadarCircularAttackRange;
    float RadarCoreVulnerabilityRadius;
    float RadarDeadZoneRadius;
    float LaunchIntervalMinSec;
    float LaunchIntervalMaxSec;
    int TotalRocketsToLaunch;

    // Статический конструктор для инициализации словаря.
    // Вызывается один раз автоматически перед первым использованием класса ConfigData.
    static ConfigData() 
    {
        keyMap = gcnew Dictionary<String^, ConfigKey>();
        keyMap->Add("rocket_speed", ConfigKey::RocketSpeed);
        keyMap->Add("distance_corner_to_center", ConfigKey::DistanceCornerToCenter);
        keyMap->Add("radar_beam_width_degrees", ConfigKey::RadarBeamWidthDegrees);
        keyMap->Add("radar_rotation_speed_dps", ConfigKey::RadarRotationSpeedDps);
        keyMap->Add("radar_max_detection_range_P", ConfigKey::RadarMaxDetectionRangeP);
        keyMap->Add("radar_circular_attack_range", ConfigKey::RadarCircularAttackRange);
        keyMap->Add("radar_core_vulnerability_radius", ConfigKey::RadarCoreVulnerabilityRadius);
        keyMap->Add("radar_dead_zone_radius", ConfigKey::RadarDeadZoneRadius);
        keyMap->Add("launch_interval_min_sec", ConfigKey::LaunchIntervalMinSec);
        keyMap->Add("launch_interval_max_sec", ConfigKey::LaunchIntervalMaxSec);
        keyMap->Add("total_rockets_to_launch", ConfigKey::TotalRocketsToLaunch);
    }

    ConfigData() 
    {
        // Значения по умолчанию остаются прежними
        RocketSpeed = 50.0f;
        DistanceCornerToCenter = 250.0f;
        RadarBeamWidthDegrees = 30.0f;
        RadarRotationSpeedDps = 45.0f;
        RadarMaxDetectionRangeP = 150.0f;
        RadarCircularAttackRange = 40.0f;
        RadarCoreVulnerabilityRadius = 10.0f;
        RadarDeadZoneRadius = 10.0f;
        LaunchIntervalMinSec = 5.0f;
        LaunchIntervalMaxSec = 8.0f;
        TotalRocketsToLaunch = 10;
        RadarBeamEffectiveRadiusR = RadarMaxDetectionRangeP / 1.5f;
    }

    // ШАГ 3 и 4: Обновленный метод с использованием switch-case 
    bool LoadFromFile(String^ filename) 
    {
        try 
        {
            StreamReader^ sr = gcnew StreamReader(filename);
            String^ line;

            while ((line = sr->ReadLine()) != nullptr) 
            {
                line = line->Trim();
                if (line->StartsWith("#") || String::IsNullOrWhiteSpace(line)) continue;

                array<String^>^ parts = line->Split('=');
                if (parts->Length == 2) {
                    String^ keyStr = parts[0]->Trim();
                    String^ value = parts[1]->Trim();

                    ConfigKey keyEnum;
                    // Ищем ключ в словаре. TryGetValue - безопасный способ.
                    if (keyMap->TryGetValue(keyStr, keyEnum)) 
                    {
                        // Если ключ найден, используем его в switch
                        switch (keyEnum) 
                        {
                        case ConfigKey::RocketSpeed:
                            RocketSpeed = Single::Parse(value, System::Globalization::CultureInfo::InvariantCulture);
                            break;
                        case ConfigKey::DistanceCornerToCenter:
                            DistanceCornerToCenter = Single::Parse(value, System::Globalization::CultureInfo::InvariantCulture);
                            break;
                        case ConfigKey::RadarBeamWidthDegrees:
                            RadarBeamWidthDegrees = Single::Parse(value, System::Globalization::CultureInfo::InvariantCulture);
                            break;
                        case ConfigKey::RadarRotationSpeedDps:
                            RadarRotationSpeedDps = Single::Parse(value, System::Globalization::CultureInfo::InvariantCulture);
                            break;
                        case ConfigKey::RadarMaxDetectionRangeP:
                            RadarMaxDetectionRangeP = Single::Parse(value, System::Globalization::CultureInfo::InvariantCulture);
                            break;
                        case ConfigKey::RadarCircularAttackRange:
                            RadarCircularAttackRange = Single::Parse(value, System::Globalization::CultureInfo::InvariantCulture);
                            break;
                        case ConfigKey::RadarCoreVulnerabilityRadius:
                            RadarCoreVulnerabilityRadius = Single::Parse(value, System::Globalization::CultureInfo::InvariantCulture);
                            break;
                        case ConfigKey::RadarDeadZoneRadius:
                            RadarDeadZoneRadius = Single::Parse(value, System::Globalization::CultureInfo::InvariantCulture);
                            break;
                        case ConfigKey::LaunchIntervalMinSec:
                            LaunchIntervalMinSec = Single::Parse(value, System::Globalization::CultureInfo::InvariantCulture);
                            break;
                        case ConfigKey::LaunchIntervalMaxSec:
                            LaunchIntervalMaxSec = Single::Parse(value, System::Globalization::CultureInfo::InvariantCulture);
                            break;
                        case ConfigKey::TotalRocketsToLaunch:
                            TotalRocketsToLaunch = Int32::Parse(value);
                            break;
                        }
                    }
                    // Если ключ не найден в словаре, мы его просто игнорируем
                }
            }
            sr->Close();

            RadarBeamEffectiveRadiusR = RadarMaxDetectionRangeP / 1.5f;
            return true;
        }
        catch (Exception^ ex) 
        {
            MessageBox::Show("Ошибка загрузки конфигурации: " + ex->Message + "\nИспользуются значения по умолчанию.", "Ошибка конфигурации");
            RadarBeamEffectiveRadiusR = RadarMaxDetectionRangeP / 1.5f;
            return false;
        }
    }
};
