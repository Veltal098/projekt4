============== Projekt 4 - Symulator Dźwigu Budowlanego ==============

Program symuluje pracę dźwigu budowlanego (żurawia), umożliwiając użytkownikowi sterowanie hakiem i układanie bloków w wieże.

Główne funkcje:
-Sterowanie dźwigiem za pomocą klawiatury
-Budowanie wież z bloków o różnych kształtach (kwadraty, trójkąty, koła)
-Każdy fundament (oznaczony odpowiednim symbolem) może pomieścić maksymalnie 3 bloki
-System ograniczeń - dźwig może podnieść tylko bloki o wadze do 100kg

Automatyzacja:
Program oferuje możliwość uruchomienia dwóch botów, które automatycznie budują wieże na dedykowanych platformach po lewej stronie ekranu. Każdy bot ma przypisaną swoją platformę (pierwszy bot - pierwsza platforma, drugi bot - druga platforma).

Uwaga techniczna: W rzadkich przypadkach przycisk startu bota może się "zaciąć". W takiej sytuacji należy zminimalizować okno programu (nie zamykając go) i ponownie je otworzyć, aby przywrócić pełną funkcjonalność.

Sterowanie:
-Strzałki: ← → ↑ ↓ - poruszanie hakiem
-Spacja: podnoszenie/opuszczanie bloku
-R: resetowanie bieżącego bloku (przydatne gdy blok jest za ciężki lub gdy chcemy zmienić jego kształt)

Kompilacja:

g++ projekt4.cpp -o crane.exe -mwindows -lgdiplus
