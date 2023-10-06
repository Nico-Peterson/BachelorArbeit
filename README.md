# BachelorArbeit

#Vorrausetzung
Um das Projekt aufzusetzen wird PlattformIO verwendet und es wird Zephyr benötigt. Es muss der Treiber für das LoRa Erweiterungsboard (eg. Ordner SX1280) an der Ensprechenden Stelle in dem Zephyr Projekt eingefügt werden. Anschließend können sowohl Anchor als auch Mobile Firmware direkt gebaut werden.

#LoRa Ranging
Um die Ranging Evaluation nachzuvollziehen wird lediglich der RangingEval Code auf zwei Geräten Benötigt. Ausgelegt ist es auf den nRF52840dk. Mit dessen buttons die verschiedenen LoRa Einstellungen durchgeschaltet werden können.

#Lokalisierung
Es wird der Entsprechende Code (mobile und anchor) auf verschiedenen Geräten benötigt und die Hardware adressen müssen angepasst sein. Während des Ablaufes gibt die Mobile Node permanent die Entsprechenden Messungen, sowie die errechnete Postition aus.

#Verwendung von Korrektur
Es wird lediglich die DataCollectionSingle verwendet. Die daraus folgenden Daten werden in das Python Skript "Korrektur" eingesetzt. Um dieses auszuführen wird Scikit benötigt. Das Skript lieftert die entsprchend korriegierten Werte. Um die Position zu errechnen wurde die Implementierung der Mobile Node verwendet.
