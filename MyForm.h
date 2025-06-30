#pragma once

#include "Config.h"   
#include "Rocket.h"     
#include "Launcher.h"   
#include "Radar.h"      

namespace radar // Объявление пространства имен для проекта, чтобы избежать конфликтов имен
{
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Collections::Generic; 

	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		// Конструктор формы
		MyForm(void)
		{
			// Этот метод автоматически генерируется дизайнером форм
			InitializeComponent();
			// Вызываем наш собственный метод для загрузки конфигурации и инициализации игры
			LoadAndInitializeGame();
		}

	protected:
		// Деструктор формы, вызывается при ее уничтожении
		~MyForm()
		{
			// Если есть компоненты, созданные дизайнером, очищаем их
			if (components)
			{
				delete components;
			}
			// Важно остановить таймер, чтобы он не пытался сработать после уничтожения формы
			if (gameTimer != nullptr && gameTimer->Enabled) 
			{
				gameTimer->Stop();
			}
		}

	private: System::Windows::Forms::Timer^ gameTimer; // Главный таймер, который управляет игровым циклом
	private:
		// Переменные для хранения состояния игры
		ConfigData^ config; // Объект с параметрами игры, загруженными из файла
		Radar^ radar; // Объект радара
		List<Launcher^>^ launchers; // Список всех пусковых установок
		List<Rocket^>^ activeRockets; // Список всех активных ракет в данный момент
		PointF worldOriginOffset; // Смещение центра игрового мира относительно левого верхнего угла окна

		// Счетчики и флаги состояния игры 
		int rocketsLaunchedCount; // Сколько всего ракет было запущено
		int rocketsInterceptedCount; // Сколько ракет было перехвачено
		bool gameOver; // Флаг, который становится true, когда игра окончена
		String^ gameStatusMessage; // Сообщение, отображаемое на экране (например, "Победа" или "Поражение")

		// Переменные для управления последовательным запуском ракет 
		float timeUntilNextPossibleLaunchSec; // Общий таймер, отсчитывающий время до следующего запуска
		bool canLaunchNextRocketFlag; // Флаг, разрешающий запуск следующей ракеты
		int nextLauncherIndex; // Индекс следующей пусковой установки, которая будет стрелять
	private: System::ComponentModel::IContainer^ components; // Контейнер для компонентов, управляемый дизайнером


	private:
		// Предикат для поиска - в текущей версии кода не используется, но может быть полезен для List::RemoveAll
		static bool Predicate_IsRocketInactiveAndIntercepted(Rocket^ r) 
		{
			return !r->IsActive && r->IsIntercepted;
		}

#pragma region Windows Form Designer generated code
		// Метод, автоматически сгенерированный дизайнером Windows Forms
		// В нем создаются и настраиваются все элементы управления на форме
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->gameTimer = (gcnew System::Windows::Forms::Timer(this->components));
			this->SuspendLayout();

			// Настройка игрового таймера
			this->gameTimer->Interval = 33; // Интервал примерно 30 кадров в секунду (1000мс / 30fps ? 33мс)
			this->gameTimer->Tick += gcnew System::EventHandler(this, &MyForm::GameTimer_Tick); // Привязка обработчика события Tick

			// Настройка самой формы
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(891, 544);
			this->DoubleBuffered = true; // Включение двойной буферизации для плавной отрисовки без мерцания
			this->Margin = System::Windows::Forms::Padding(2);
			this->Name = L"MyForm";
			this->Text = L"Радар";
			// Привязка обработчиков событий формы
			this->Load += gcnew System::EventHandler(this, &MyForm::MyForm_Load);
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::MyForm_Paint);
			this->Resize += gcnew System::EventHandler(this, &MyForm::MyForm_Resize);
			this->ResumeLayout(false);

		}
#pragma endregion

	private:
		// Метод для инициализации или перезапуска игры
		void LoadAndInitializeGame() 
		{
			// Создаем объект конфигурации и загружаем данные из файла
			config = gcnew ConfigData();
			config->LoadFromFile("settings.txt"); // Файл должен лежать в папке с exe-файлом

			// Определяем центр окна как начало мировых координат (0,0)
			worldOriginOffset = PointF(this->ClientSize.Width / 2.0f, this->ClientSize.Height / 2.0f);

			// Создаем радар в центре игрового мира с параметрами из конфига
			radar = gcnew Radar(PointF(0, 0), config->RadarRotationSpeedDps, config->RadarBeamWidthDegrees,
				config->RadarMaxDetectionRangeP, config->RadarBeamEffectiveRadiusR,
				config->RadarCircularAttackRange, config->RadarCoreVulnerabilityRadius,
				config->RadarDeadZoneRadius);

			// Создаем пусковые установки по углам квадрата вокруг центра
			launchers = gcnew List<Launcher^>();
			// Вычисляем смещение по X и Y для расположения установок
			float d = (float)(config->DistanceCornerToCenter / Math::Sqrt(2.0));
			launchers->Add(gcnew Launcher(PointF(-d, -d), 0, config->LaunchIntervalMinSec, config->LaunchIntervalMaxSec)); // Верхняя левая
			launchers->Add(gcnew Launcher(PointF(d, -d), 1, config->LaunchIntervalMinSec, config->LaunchIntervalMaxSec));  // Верхняя правая
			launchers->Add(gcnew Launcher(PointF(d, d), 2, config->LaunchIntervalMinSec, config->LaunchIntervalMaxSec));   // Нижняя правая
			launchers->Add(gcnew Launcher(PointF(-d, d), 3, config->LaunchIntervalMinSec, config->LaunchIntervalMaxSec));  // Нижняя левая

			// Инициализируем список для активных ракет
			activeRockets = gcnew List<Rocket^>();

			// Сбрасываем все игровые счетчики и флаги в начальное состояние
			rocketsLaunchedCount = 0;
			rocketsInterceptedCount = 0;
			gameOver = false;
			gameStatusMessage = "Игра началась, защищайте радар";

			// Сбрасываем таймеры запуска ракет
			timeUntilNextPossibleLaunchSec = 0; // Первая ракета может стартовать немедленно
			canLaunchNextRocketFlag = true;
			nextLauncherIndex = 0; // Начинаем с первой пусковой установки

			// Запускаем игровой таймер, если он существует
			if (gameTimer) gameTimer->Start();
		}

		// Обработчик события загрузки формы
		System::Void MyForm_Load(System::Object^ sender, System::EventArgs^ e) 
		{
			// Метод LoadAndInitializeGame() уже вызывается в конструкторе, так что здесь ничего делать не нужно
		}

		// Обработчик события изменения размера окна
		System::Void MyForm_Resize(System::Object^ sender, System::EventArgs^ e) 
		{
			// Пересчитываем положение центра мира при изменении размера окна
			worldOriginOffset = PointF(this->ClientSize.Width / 2.0f, this->ClientSize.Height / 2.0f);
			// Вызываем принудительную перерисовку окна
			this->Invalidate();
		}

		// Главный игровой цикл, вызывается по таймеру
		System::Void GameTimer_Tick(System::Object^ sender, System::EventArgs^ e) 
		{
			// Блок проверки окончания игры 
			if (gameOver) 
			{
				gameTimer->Stop(); // Останавливаем игровой цикл
				this->Invalidate(); // Перерисовываем экран, чтобы показать финальное сообщение

				// Показываем модальное окно с результатом игры
				System::Windows::Forms::DialogResult result = MessageBox::Show(
					this, // Родительское окно
					gameStatusMessage, // Текст сообщения
					"Игра окончена", // Заголовок окна
					MessageBoxButtons::OK, // Одна кнопка OK
					MessageBoxIcon::Information // Иконка
				);

				// Если пользователь нажал OK, перезапускаем игру
				if (result == System::Windows::Forms::DialogResult::OK) 
				{
					LoadAndInitializeGame(); // Этот метод сбросит все и начнет игру заново
				}
				else // Если пользователь закрыл окно, не нажимая OK
				{
					this->Close(); // Закрываем приложение
				}
				return; // Выходим из текущего тика, так как игра либо перезапущена, либо закрыта
			}

			// Вычисляем время, прошедшее с последнего кадра, в секундах
			float deltaTime = (float)gameTimer->Interval / 1000.0f;

			// 1, Обновление состояния радара (вращение)
			radar->Update(deltaTime);

			// 2, Обновление собственных таймеров пусковых установок
			for each(Launcher ^ launcher in launchers) 
			{
				launcher->UpdateOwnTimer(deltaTime);
			}

			// 3, Логика последовательного запуска ракет
			// Уменьшаем общий таймер ожидания
			if (timeUntilNextPossibleLaunchSec > 0) 
			{
				timeUntilNextPossibleLaunchSec -= deltaTime;
			}
			else {
				// Когда таймер истек, разрешаем запуск
				canLaunchNextRocketFlag = true;
			}

			// Если можно запускать и еще не все ракеты запущены
			if (canLaunchNextRocketFlag && rocketsLaunchedCount < config->TotalRocketsToLaunch) 
			{
				// Выбираем текущую пусковую установку
				Launcher^ currentLauncher = launchers[nextLauncherIndex];

				// Проверяем, готова ли стрелять именно эта установка (ее личный таймер)
				if (currentLauncher->TimeToNextLaunchSec <= 0) {
					// Если готова, производим запуск
					Rocket^ newRocket = currentLauncher->Fire(radar->Position, config->RocketSpeed);
					activeRockets->Add(newRocket);
					rocketsLaunchedCount++;

					// Устанавливаем общую задержку до следующего ВОЗМОЖНОГО запуска
					timeUntilNextPossibleLaunchSec = (float)Launcher::SharedRandom->NextDouble() *
						(config->LaunchIntervalMaxSec - config->LaunchIntervalMinSec) +
						config->LaunchIntervalMinSec;
					canLaunchNextRocketFlag = false; // Запрещаем запуск до истечения таймера

					// Переходим к следующей установке по кругу
					nextLauncherIndex = (nextLauncherIndex + 1) % launchers->Count;
				}
			}


			// 4, Обновление ракет и проверка столкновений
			// Идем по списку с конца, чтобы безопасно удалять элементы
			for (int i = activeRockets->Count - 1; i >= 0; i--) 
			{
				Rocket^ rocket = activeRockets[i];
				if (!rocket->IsActive) // Пропускаем уже неактивные ракеты
				{ 
					continue;
				}

				rocket->Update(deltaTime); // Обновляем позицию ракеты

				// Проверка на уничтожение радара
				if (rocket->GetDistanceTo(radar->Position) <= radar->CoreVulnerabilityRadius) 
				{
					radar->IsDestroyed = true;
					gameOver = true;
					gameStatusMessage = "Радар уничтожен";
					break; // Немедленно выходим из цикла, игра окончена
				}

				// Попытка перехвата ракеты радаром
				if (!radar->IsDestroyed && radar->DetectAndIntercept(rocket)) 
				{
					rocketsInterceptedCount++;
					// Флаг IsActive станет false внутри метода DetectAndIntercept
				}
			}

			// Если игра закончилась из-за уничтожения радара, выходим из этого тика
			if (gameOver) 
			{
				this->Invalidate(); // Запрашиваем перерисовку, чтобы показать уничтоженный радар
				return;
			}

			// Удаляем все неактивные ракеты из списка
			// (проверка отдельным циклом для ясности)
			for (int i = activeRockets->Count - 1; i >= 0; i--)
			{
				if (!activeRockets[i]->IsActive) 
				{
					activeRockets->RemoveAt(i);
				}
			}


			// 5, Проверка условий победы или поражения (если радар еще цел)
			if (!gameOver) 
			{
				// Если все запланированные ракеты запущены и на поле больше нет активных ракет
				if (rocketsLaunchedCount >= config->TotalRocketsToLaunch && activeRockets->Count == 0) 
				{
					// Все ракеты были сбиты - это победа
					if (rocketsInterceptedCount == config->TotalRocketsToLaunch) 
					{
						gameStatusMessage = "ПОБЕДА, Все ракеты перехвачены";
					}
					else // Некоторые ракеты пролетели мимо, но не попали в ядро
					{ 
						gameStatusMessage = String::Format("ЗАЩИТА ПРОВАЛЕНА, Запущено: {0}, Перехвачено: {1}", config->TotalRocketsToLaunch, rocketsInterceptedCount);
					}
					gameOver = true; // В любом случае игра окончена
				}
			}
			// В конце каждого кадра запрашиваем перерисовку формы
			this->Invalidate();
		}

		// Метод отрисовки, вызывается каждый раз, когда нужно перерисовать окно
		System::Void MyForm_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) 
		{
			// Получаем объект Graphics для рисования
			Graphics^ g = e->Graphics;
			// Включаем сглаживание для более красивой графики
			g->SmoothingMode = System::Drawing::Drawing2D::SmoothingMode::AntiAlias;

			// Очищаем экран черным цветом
			g->Clear(Color::Black);

			// Рисуем радар, если он был создан
			if (radar != nullptr) 
			{
				radar->Draw(g, worldOriginOffset);
			}

			// Рисуем пусковые установки, если они были созданы
			if (launchers != nullptr) 
			{
				for each(Launcher ^ launcher in launchers) 
				{
					launcher->Draw(g, worldOriginOffset);
				}
			}

			// Рисуем активные ракеты
			if (activeRockets != nullptr) 
			{
				// Копируем список, чтобы избежать ошибок, если он изменится во время отрисовки (маловероятно, но безопасно)
				List<Rocket^>^ rocketsToDraw = gcnew List<Rocket^>(activeRockets);
				for each(Rocket ^ rocket in rocketsToDraw) 
				{
					// Рисуем только активные или только что перехваченные ракеты
					if (rocket->IsActive || (rocket->IsIntercepted && !rocket->IsActive)) 
					{
						rocket->Draw(g, worldOriginOffset);
					}
				}
			}

			// Выводим на экран текстовую информацию о состоянии игры
			String^ statusText = String::Format(
				"Запущено ракет: {0}/{1}\nПерехвачено: {2}\nСостояние радара: {3}\n{4}",
				rocketsLaunchedCount, config->TotalRocketsToLaunch,
				rocketsInterceptedCount,
				(radar != nullptr && radar->IsDestroyed) ? "УНИЧТОЖЕН" : "РАБОТАЕТ",
				gameStatusMessage
			);
			g->DrawString(statusText, this->Font, Brushes::LightGreen, 10, 10);
		}
	}; // конец класса MyForm
#pragma endregion
}; // конец пространства имен radar
