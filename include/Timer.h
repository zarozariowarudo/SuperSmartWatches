#include <Arduino.h>
#include <time.h>

class Timer
{
    private:
        time_t target_time;
        bool is_active = false;
        long remaining_sec;

        int beep_interval = 250;
        unsigned long beep_prev;
        int num_of_beeps = 5;
        int beep_counter = 0;
        bool beeped = false;
        byte beep_pin = 12;

        int minutes = 5;

    public:
        void startTimer()
        {
            time_t now;
            time(&now);
            target_time = now + (minutes * 60);
            is_active = true;
            beeped = false;
            beep_counter = 0;
            beep_prev = 0;

            pinMode(beep_pin, OUTPUT);
            digitalWrite(beep_pin, LOW);
        }

        void tick()
        {
            if (is_active == false) return;

            time_t now;
            time(&now);
            remaining_sec = target_time - now;

            if (remaining_sec <= 0)
            {
                remaining_sec = 0;

                if (!beeped)
                {
                    if (millis() - beep_prev >= beep_interval)
                    {
                        beep_prev = millis();
                        
                        // Инвертируем состояние пина (ВКЛ / ВЫКЛ)
                        digitalWrite(beep_pin, !digitalRead(beep_pin));
                        beep_counter++;

                        // Если отпищали нужное количество полупериодов (вкл + выкл)
                        if (beep_counter >= num_of_beeps * 2)
                        {
                            digitalWrite(beep_pin, LOW); // Гарантированно выключаем баззер
                            beeped = true;
                            is_active = false;
                        }
                    }
                }
            }
        }

        String getRemaining()
        {
            if ((remaining_sec >= 0) && (is_active))
            {
                int minutes_left = remaining_sec / 60;
                int seconds_left = remaining_sec % 60;

                char buffer[10];
                snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes_left, seconds_left); 

                return String(buffer);
            }
            else
            {
                char buffer[10];
                snprintf(buffer, sizeof(buffer), "%02d:00", minutes);
                return String(buffer);
            }
        }

        String getRemainingMin()
        {
            if (remaining_sec >= 0 && is_active)
            {
                char buf[4];
                snprintf(buf, sizeof(buf), "%02d", (int)(remaining_sec / 60));
                return String(buf);
            }
            return "--";
        }

        String getRemainingSec()
        {
            if (remaining_sec >= 0 && is_active)
            {
                char buf[4];
                snprintf(buf, sizeof(buf), "%02d", (int)(remaining_sec % 60));
                return String(buf);
            }
            return "--";
        }

        void resetTimer()
        {
            remaining_sec = 0;
            target_time = 0;
            is_active = 0;
            beeped = true;
            digitalWrite(beep_pin, LOW);
        }

        void addMnute()
        {
            if (!is_active) 
            {
                minutes += 5;
                if (minutes > 95) minutes = 95; // Ограничение сверху
            }
        }

        void minusMinute()
        {
            if (!is_active) 
            {
                minutes -= 5;
                if (minutes < 1) minutes = 1; // Защита от ухода в 0 и минус
            }
        }

        bool isActive() const { return is_active; }
};