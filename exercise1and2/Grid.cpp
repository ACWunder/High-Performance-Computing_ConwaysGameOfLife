#include "Grid.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string.h>
#include <random>
#include <ctime>
#include "opencl.hpp"
#include "utilities.hpp"

// Der folgende Abschnitt inkludiert systemabhängige Header-Dateien,
// um Plattformunterschiede zwischen Windows und Unix-basierten Systemen auszugleichen.
#ifdef _WIN32
#include <windows.h>  // Windows-spezifische Funktionen
#else
#include <unistd.h>   // Unix-spezifische Funktionen (z.B. sleep)
#endif

// Dies ist der Pfad zur OpenCL-Kernel-Datei, die den Code für die parallele Ausführung enthält.
const std::string kernel_path = "/home/users8/acgl/s4948503/Documents/abschluss3/game_of_life.cl";

// Konstruktor ohne Parameter: Initialisiert ein leeres Grid-Objekt mit Höhe und Breite auf 0,
// und aktiviert die Druckfunktion standardmäßig.
Grid::Grid() : height(0), width(0), printEnabled(true) {}

// Konstruktor mit Parametern: Initialisiert ein Grid mit gegebener Höhe (h) und Breite (w).
// Die aktuellen und nächsten Generationen werden als zweidimensionale Vektoren von booleschen Werten initialisiert.
// Der printEnabled-Status wird auf true gesetzt.
Grid::Grid(int h, int w) 
    : height(h), 
      width(w), 
      currentGeneration(h, std::vector<bool>(w, false)),  // Initialisiere currentGeneration mit "false" (alle Zellen tot)
      nextGeneration(h, std::vector<bool>(w, false)),    // Initialisiere nextGeneration ebenfalls mit "false"
      printEnabled(true) {}

// Konvertiert einen eindimensionalen Index in ein zweidimensionales (x, y) Paar.
// Dies ist nützlich, wenn die Zellen in einem eindimensionalen Array gespeichert werden.
std::pair<int, int> Grid::to2D(int p) const {
    // p / width gibt die Zeile (x) und p % width gibt die Spalte (y) zurück.
    return { p / width, p % width };
}

// Lädt ein Zellenmuster aus einer Datei, um das Gitter (Grid) zu initialisieren.
// Die Datei muss die Höhe, Breite und das Startmuster der Zellen enthalten.
void Grid::initializePattern(const std::string &filename) {
    std::ifstream file(filename);  // Öffne die Datei zum Lesen
    if (!file.is_open()) {  // Überprüfen, ob die Datei erfolgreich geöffnet wurde
        // Fehlerbehandlung, falls die Datei nicht geöffnet werden konnte
        std::cerr << "Attempting to open file at: " << filename << std::endl;
        std::cerr << "Error opening file!" << std::endl;
        std::cerr << "Error code: " << strerror(errno) << std::endl;
        return;  // Beende die Funktion, da die Datei nicht gelesen werden kann
    }

    // Lese die Höhe und Breite des Gitters aus der Datei
    file >> height >> width;

    // Passe die Größe der currentGeneration und nextGeneration Vektoren entsprechend der neuen Höhe und Breite an
    currentGeneration.resize(height, std::vector<bool>(width, false));
    nextGeneration.resize(height, std::vector<bool>(width, false));

    // Lese die Zellzustände aus der Datei und setze die entsprechenden Zellen in currentGeneration
    for (int i = 0; i < height; ++i) {  // Schleife über alle Zeilen
        for (int j = 0; j < width; ++j) {  // Schleife über alle Spalten
            int cell;
            file >> cell;  // Lese den Zellzustand (0 oder 1)
            currentGeneration[i][j] = (cell == 1);  // Setze den Zustand in currentGeneration (true für lebend, false für tot)
        }
    }

    file.close();  // Schließe die Datei, da sie nicht mehr benötigt wird
}

// Gibt das aktuelle Gitter auf der Konsole aus.
// Lebende Zellen werden als 'O' und tote Zellen als '.' dargestellt.
void Grid::print() const {
    for (int i = 0; i < height; ++i) {  // Schleife über alle Zeilen
        for (int j = 0; j < width; ++j) {  // Schleife über alle Spalten
            // Ausgabe des aktuellen Zellzustands: 'O' für lebend, '.' für tot
            std::cout << (currentGeneration[i][j] ? 'O' : '.');
        }
        std::cout << std::endl;  // Zeilenumbruch nach jeder Zeile
    }
}

// Führt das Spiel über eine bestimmte Anzahl von Generationen aus und misst die dafür benötigte Zeit.
// Zwischen den Generationen wird eine Verzögerung eingeführt, und das Ergebnis kann gedruckt werden.
long long Grid::run(int generations, int delay_ms) {
    // Erfasse den Startzeitpunkt der Berechnung
    auto start_time = std::chrono::high_resolution_clock::now();

    // Füge einige Muster zum Testen in das Gitter ein
    addGlider(25, 25);        // Füge einen Glider an Position (25, 25) hinzu
    addToad(100, 100);        // Füge ein Toad an Position (100, 100) hinzu
    addBeacon(125, 125);      // Füge ein Beacon an Position (125, 125) hinzu
    addRPentomino(150, 150);  // Füge ein R-Pentomino an Position (150, 150) hinzu

    // Führe die Simulation für die angegebene Anzahl von Generationen durch
    for (int step = 0; step < generations; ++step) {
        if (printEnabled) {  // Überprüfen, ob das Drucken aktiviert ist
            clearScreen();  // Bildschirm löschen (für bessere Lesbarkeit)
            std::cout << "Generation " << step + 1 << ":\n";
            print();  // Das aktuelle Gitter ausgeben
        }
        evolve_cpu();  // Führe die Evolution der Zellen auf der CPU durch

        if (is_stable()) {  // Überprüfen, ob ein stabiler Zustand erreicht ist
            std::cout << "\nStable configuration detected at generation " << step + 1 << ".\n";
            break;  // Beende die Schleife vorzeitig, wenn das Gitter stabil ist
        }

        // Füge eine Verzögerung zwischen den Generationen ein, um die Ausgabe zu verlangsamen
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    // Erfasse den Endzeitpunkt der Berechnung
    auto end_time = std::chrono::high_resolution_clock::now();

    // Berechne die verstrichene Zeit in Millisekunden
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Ausgabe der Gesamtzeit für die Berechnung der Generationen
    std::cout << "\nTotal calculation time (scalar) for " << generations << " generations: " << duration.count() << " ms\n";

    // Rückgabe der berechneten Dauer
    return duration.count();
}

long long Grid::run_with_opencl(int generations, int delay_ms) {
    // Diese Funktion führt das Spiel über eine bestimmte Anzahl von Generationen aus, nutzt dabei aber OpenCL zur Parallelisierung.
    // Die benötigte Zeit für die Ausführung wird gemessen und zurückgegeben.

    // Erfasse den Startzeitpunkt der Berechnung
    auto start_time = std::chrono::high_resolution_clock::now();

    // Füge einige Muster zum Testen in das Gitter ein
    addGlider(25, 25);        // Füge einen Glider an Position (25, 25) hinzu
    addToad(100, 100);        // Füge ein Toad an Position (100, 100) hinzu
    addBeacon(125, 125);      // Füge ein Beacon an Position (125, 125) hinzu
    addRPentomino(150, 150);  // Füge ein R-Pentomino an Position (150, 150) hinzu

    // Führe die Simulation für die angegebene Anzahl von Generationen durch
    for (int step = 0; step < generations; ++step) {
        if (printEnabled) {  // Überprüfen, ob das Drucken aktiviert ist
            clearScreen();  // Bildschirm löschen (für bessere Lesbarkeit)
            std::cout << "Generation " << step + 1 << ":\n";
            print();  // Das aktuelle Gitter ausgeben
        }
        evolve();  // Führe die Evolution der Zellen mit OpenCL durch

        if (is_stable()) {  // Überprüfen, ob ein stabiler Zustand erreicht ist
            std::cout << "\nStable configuration detected at generation " << step + 1 << ".\n";
            break;  // Beende die Schleife vorzeitig, wenn das Gitter stabil ist
        }

        // Füge eine Verzögerung zwischen den Generationen ein, um die Ausgabe zu verlangsamen
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    // Erfasse den Endzeitpunkt der Berechnung
    auto end_time = std::chrono::high_resolution_clock::now();

    // Berechne die verstrichene Zeit in Millisekunden
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Ausgabe der Gesamtzeit für die Berechnung der Generationen mit OpenCL
    std::cout << "\nTotal calculation time for " << generations << " generations (OpenCL): " << duration.count() << " ms\n";

    // Rückgabe der berechneten Dauer
    return duration.count();
}

bool Grid::load(const std::string &filename) {
    // Diese Funktion lädt den Zustand des Gitters aus einer Datei und setzt die Höhe und Breite des Gitters entsprechend.
    std::ifstream file(filename);  // Öffne die Datei zum Lesen
    if (!file.is_open()) {  // Überprüfen, ob die Datei erfolgreich geöffnet wurde
        std::cerr << "Error opening file!" << std::endl;
        return false;  // Rückgabe false, wenn die Datei nicht geöffnet werden konnte
    }

    // Lese die Höhe und Breite des Gitters aus der Datei
    file >> height >> width;

    // Passe die Größe der currentGeneration und nextGeneration Vektoren entsprechend der neuen Höhe und Breite an
    currentGeneration.resize(height, std::vector<bool>(width, false));
    nextGeneration.resize(height, std::vector<bool>(width, false));

    // Lese die Zellzustände aus der Datei und setze die entsprechenden Zellen in currentGeneration
    for (int i = 0; i < height; ++i) {  // Schleife über alle Zeilen
        for (int j = 0; j < width; ++j) {  // Schleife über alle Spalten
            int cell;
            file >> cell;  // Lese den Zellzustand (0 oder 1)
            currentGeneration[i][j] = (cell == 1);  // Setze den Zustand in currentGeneration (true für lebend, false für tot)
        }
    }

    file.close();  // Schließe die Datei, da sie nicht mehr benötigt wird
    return true;  // Rückgabe true, wenn der Ladevorgang erfolgreich war
}

bool Grid::save(const std::string &filename) const {
    // Diese Funktion speichert den aktuellen Zustand des Gitters in einer Datei, einschließlich der Höhe und Breite.
    std::ofstream file(filename);  // Öffne die Datei zum Schreiben
    if (!file.is_open()) {  // Überprüfen, ob die Datei erfolgreich geöffnet wurde
        std::cerr << "Error opening file!" << std::endl;
        return false;  // Rückgabe false, wenn die Datei nicht geöffnet werden konnte
    }

    // Schreibe die Höhe und Breite des Gitters in die Datei
    file << height << " " << width << "\n";
    
    // Schleife über alle Zellen und schreibe deren Zustand (0 oder 1) in die Datei
    for (int i = 0; i < height; ++i) {  // Schleife über alle Zeilen
        for (int j = 0; j < width; ++j) {  // Schleife über alle Spalten
            file << (currentGeneration[i][j] ? 1 : 0) << " ";  // Schreibe 1 für lebende Zellen und 0 für tote Zellen
        }
        file << "\n";  // Zeilenumbruch nach jeder Zeile
    }

    file.close();  // Schließe die Datei, da sie nicht mehr benötigt wird
    return true;  // Rückgabe true, wenn der Speichervorgang erfolgreich war
}

void Grid::setSize(int h, int w) {
    // Diese Funktion setzt die Größe des Gitters auf die angegebenen Werte und initialisiert die Zellzustände.
    height = h;  // Setze die Höhe des Gitters
    width = w;   // Setze die Breite des Gitters

    // Passe die Größe der currentGeneration und nextGeneration Vektoren entsprechend der neuen Höhe und Breite an
    currentGeneration.resize(height, std::vector<bool>(width, false));
    nextGeneration.resize(height, std::vector<bool>(width, false));
}

int Grid::getHeight() const { 
    // Diese Funktion gibt die Höhe des Gitters zurück
    return height; 
}

int Grid::getWidth() const { 
    // Diese Funktion gibt die Breite des Gitters zurück
    return width; 
}

void Grid::setCell(int x, int y, bool state) {
    // Diese Funktion setzt den Zustand einer bestimmten Zelle im Gitter, basierend auf den gegebenen x- und y-Koordinaten.
    if (x >= 0 && x < height && y >= 0 && y < width) {
        // Überprüfen, ob die angegebenen Koordinaten innerhalb der Grenzen des Gitters liegen
        currentGeneration[x][y] = state;  // Setze den Zustand der Zelle
    } else {
        std::cerr << "Error: Coordinates out of bounds.\n";  // Fehlerausgabe, wenn die Koordinaten außerhalb der Grenzen liegen
    }
}

void Grid::setCell(int p, bool state) {
    // Diese Funktion setzt den Zustand einer bestimmten Zelle im Gitter, basierend auf einem eindimensionalen Index.
    auto coords = to2D(p);  // Konvertiere den eindimensionalen Index in x- und y-Koordinaten
    int x = coords.first;
    int y = coords.second;
    
    if (x >= 0 && x < height && y >= 0 && y < width) {
        // Überprüfen, ob die berechneten Koordinaten innerhalb der Grenzen des Gitters liegen
        currentGeneration[x][y] = state;  // Setze den Zustand der Zelle
    } else {
        std::cerr << "Error: Index out of bounds.\n";  // Fehlerausgabe, wenn der Index außerhalb der Grenzen liegt
    }
}
bool Grid::getCell(int x, int y) const {
    // Diese Funktion gibt den Zustand einer bestimmten Zelle im Gitter zurück, basierend auf den gegebenen x- und y-Koordinaten.
    if (x >= 0 && x < height && y >= 0 && y < width) {
        // Überprüfen, ob die angegebenen Koordinaten innerhalb der Grenzen des Gitters liegen.
        return currentGeneration[x][y];  // Rückgabe des Zustands der Zelle (true für lebend, false für tot).
    } else {
        std::cerr << "Error: Coordinates out of bounds.\n";  // Fehlerausgabe, wenn die Koordinaten außerhalb der Grenzen liegen.
        return false;  // Rückgabe false, wenn die Koordinaten ungültig sind.
    }
}

bool Grid::getCell(int p) const {
    // Diese Funktion gibt den Zustand einer bestimmten Zelle im Gitter zurück, basierend auf einem eindimensionalen Index.
    auto coords = to2D(p);  // Konvertiere den eindimensionalen Index in x- und y-Koordinaten.
    int x = coords.first;
    int y = coords.second;

    if (x >= 0 && x < height && y >= 0 && y < width) {
        // Überprüfen, ob die berechneten Koordinaten innerhalb der Grenzen des Gitters liegen.
        return currentGeneration[x][y];  // Rückgabe des Zustands der Zelle (true für lebend, false für tot).
    } else {
        std::cerr << "Error: Index out of bounds.\n";  // Fehlerausgabe, wenn der Index außerhalb der Grenzen liegt.
        return false;  // Rückgabe false, wenn der Index ungültig ist.
    }
}

void Grid::addGlider(int x, int y) {
    // Diese Funktion fügt ein Glider-Muster in das Gitter ein, beginnend bei den angegebenen x- und y-Koordinaten.
    std::vector<std::pair<int, int>> glider{{0, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2}};
    // Definiert die relativen Positionen der Zellen, die das Glider-Muster bilden.

    for (auto& p : glider) {
        setCell(x + p.first, y + p.second, true);  // Setze jede Zelle im Glider-Muster auf "lebend".
    }
}

void Grid::addToad(int x, int y) {
    // Diese Funktion fügt ein Toad-Muster in das Gitter ein, beginnend bei den angegebenen x- und y-Koordinaten.
    std::vector<std::pair<int, int>> toad{{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 1}, {1, 2}};
    // Definiert die relativen Positionen der Zellen, die das Toad-Muster bilden.

    for (auto& p : toad) {
        setCell(x + p.first, y + p.second, true);  // Setze jede Zelle im Toad-Muster auf "lebend".
    }
}

void Grid::addBeacon(int x, int y) {
    // Diese Funktion fügt ein Beacon-Muster in das Gitter ein, beginnend bei den angegebenen x- und y-Koordinaten.
    std::vector<std::pair<int, int>> beacon{{0, 0}, {0, 1}, {1, 0}, {1, 1}, {2, 2}, {2, 3}, {3, 2}, {3, 3}};
    // Definiert die relativen Positionen der Zellen, die das Beacon-Muster bilden.

    for (auto& p : beacon) {
        setCell(x + p.first, y + p.second, true);  // Setze jede Zelle im Beacon-Muster auf "lebend".
    }
}

void Grid::addRPentomino(int x, int y) {
    // Diese Funktion fügt ein R-Pentomino-Muster in das Gitter ein, beginnend bei den angegebenen x- und y-Koordinaten.
    std::vector<std::pair<int, int>> rPentomino{{0, 1}, {0, 2}, {1, 0}, {1, 1}, {2, 1}};
    // Definiert die relativen Positionen der Zellen, die das R-Pentomino-Muster bilden.

    for (auto& p : rPentomino) {
        setCell(x + p.first, y + p.second, true);  // Setze jede Zelle im R-Pentomino-Muster auf "lebend".
    }
}

void Grid::randomize() {
    // Diese Funktion fügt zufällig ein Muster (Glider, Toad, Beacon oder R-Pentomino) an einer zufälligen Position im Gitter ein.
    
    // Initialisiere den Zufallszahlengenerator mit der aktuellen Zeit als Seed.
    static std::mt19937 rng(std::time(nullptr));
    
    // Erstelle Zufallsverteilungen für das Muster und die x- und y-Koordinaten.
    std::uniform_int_distribution<int> patternDist(0, 3);  // Verteilung zur zufälligen Auswahl eines Musters (0 bis 3).
    std::uniform_int_distribution<int> xDist(0, height - 1);  // Verteilung zur zufälligen Auswahl der x-Koordinate.
    std::uniform_int_distribution<int> yDist(0, width - 1);   // Verteilung zur zufälligen Auswahl der y-Koordinate.

    // Zufällige Auswahl eines Musters.
    int pattern = patternDist(rng);

    // Generiere zufällige Koordinaten, stelle sicher, dass das Muster im Gitter passt.
    int x = xDist(rng);
    int y = yDist(rng);

    switch(pattern) {
        case 0:  // Glider
            if (x + 2 >= height) x = height - 3;  // Anpassung der x-Koordinate, falls der Glider außerhalb des Gitters wäre.
            if (y + 2 >= width) y = width - 3;    // Anpassung der y-Koordinate, falls der Glider außerhalb des Gitters wäre.
            addGlider(x, y);  // Füge den Glider an der berechneten Position hinzu.
            break;
        case 1:  // Toad
            if (x + 1 >= height) x = height - 2;  // Anpassung der x-Koordinate, falls das Toad außerhalb des Gitters wäre.
            if (y + 3 >= width) y = width - 4;    // Anpassung der y-Koordinate, falls das Toad außerhalb des Gitters wäre.
            addToad(x, y);  // Füge das Toad an der berechneten Position hinzu.
            break;
        case 2:  // Beacon
            if (x + 3 >= height) x = height - 4;  // Anpassung der x-Koordinate, falls das Beacon außerhalb des Gitters wäre.
            if (y + 3 >= width) y = width - 4;    // Anpassung der y-Koordinate, falls das Beacon außerhalb des Gitters wäre.
            addBeacon(x, y);  // Füge das Beacon an der berechneten Position hinzu.
            break;
        case 3:  // R-Pentomino
            if (x + 2 >= height) x = height - 3;  // Anpassung der x-Koordinate, falls das R-Pentomino außerhalb des Gitters wäre.
            if (y + 2 >= width) y = width - 3;    // Anpassung der y-Koordinate, falls das R-Pentomino außerhalb des Gitters wäre.
            addRPentomino(x, y);  // Füge das R-Pentomino an der berechneten Position hinzu.
            break;
    }
}

void Grid::setPrintEnabled(bool enabled) {
    // Diese Funktion aktiviert oder deaktiviert die Druckfunktion, abhängig vom übergebenen Parameter.
    printEnabled = enabled;  // Setze den Wert von printEnabled auf den übergebenen Wert.
}

int Grid::countLiveNeighbors(int x, int y) const {
    // Diese Funktion zählt die Anzahl der lebenden Nachbarn einer Zelle an den gegebenen x- und y-Koordinaten.
    
    int count = 0;  // Zähler für die lebenden Nachbarn, initialisiert mit 0.

    // Schleifen über die benachbarten Zellen (inklusive diagonaler Nachbarn).
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx != 0 || dy != 0) {  // Ignoriere die Zelle selbst (dx == 0 und dy == 0).
                // Berechne die Koordinaten des Nachbarn, wobei toroidale Randbedingungen angewendet werden.
                int nx = (x + dx + height) % height;
                int ny = (y + dy + width) % width;
                
                // Inkrementiere den Zähler, wenn der Nachbar lebendig ist.
                count += currentGeneration[nx][ny] ? 1 : 0;
            }
        }
    }

    return count;  // Rückgabe der Anzahl der lebenden Nachbarn.
}

void Grid::evolve_cpu() {
    // Diese Funktion berechnet die nächste Generation des Gitters auf der CPU,
    // indem sie die Regeln des "Game of Life" für jede Zelle anwendet.

    // Doppelte Schleifen über jede Zelle des Gitters (x, y).
    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            int count = 0;  // Zähler für die lebenden Nachbarn, initialisiert mit 0.
            
            // Schleifen über die benachbarten Zellen, einschließlich diagonaler Nachbarn.
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (dx != 0 || dy != 0) {  // Überspringe die Zelle selbst (dx == 0 und dy == 0).
                        // Berechne die Koordinaten des Nachbarn, wobei toroidale Randbedingungen angewendet werden.
                        int nx = (x + dx + height) % height;
                        int ny = (y + dy + width) % width;
                        // Inkrementiere den Zähler, wenn der Nachbar lebendig ist.
                        count += (currentGeneration[nx][ny] == true);
                    }
                }
            }

            // Wende die Regeln des "Game of Life" an:
            // Eine lebende Zelle bleibt am Leben, wenn sie 2 oder 3 lebende Nachbarn hat.
            // Eine tote Zelle wird wieder lebendig, wenn sie genau 3 lebende Nachbarn hat.
            nextGeneration[x][y] = (currentGeneration[x][y] && (count == 2 || count == 3)) || (!currentGeneration[x][y] && count == 3);
        }
    }

    // Tausche die aktuelle und die nächste Generation, um die Berechnung der nächsten Generation zu ermöglichen.
    currentGeneration.swap(nextGeneration);
}

void Grid::evolve() {
    // Diese Funktion berechnet die nächste Generation des Gitters mit Hilfe von OpenCL,
    // wobei die Berechnungen auf einer GPU oder einem anderen OpenCL-Gerät parallel ausgeführt werden.

    cl::Context context(CL_DEVICE_TYPE_DEFAULT);  // Erstelle einen OpenCL-Kontext für das Standardgerät.
    
    std::string kernel_code = util::loadProgram(kernel_path);  // Lade den OpenCL-Kernelcode aus einer Datei.
    cl::Program::Sources sources;  // Erstelle ein Quellenobjekt für den Kernelcode.
    sources.push_back({kernel_code.c_str(), kernel_code.length()});  // Füge den Kernelcode als Quelle hinzu.

    cl::Program program(context, sources);  // Erstelle ein OpenCL-Programm aus den Quellen im gegebenen Kontext.

    auto devices = context.getInfo<CL_CONTEXT_DEVICES>();  // Hole die Liste der Geräte im Kontext.
    if (program.build(devices) != CL_SUCCESS) {  // Baue das Programm für die Geräte und überprüfe auf Fehler.
        for (const auto& device : devices) {
            // Wenn ein Fehler auftritt, gebe eine Fehlermeldung für jedes Gerät aus.
            std::cerr << "Error building the kernel for device: "
                      << device.getInfo<CL_DEVICE_NAME>() << std::endl;
            std::cerr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        }
        return;  // Beende die Funktion, wenn der Build fehlschlägt.
    }

    cl::Kernel kernel(program, "evolve");  // Erstelle einen Kernel aus dem Programm, der die Funktion "evolve" ausführt.

    // Erstelle OpenCL-Puffer für die aktuellen und nächsten Generationen sowie für Debug-Informationen.
    cl::Buffer buffer_current(context, CL_MEM_READ_WRITE, height * width * sizeof(char));
    cl::Buffer buffer_next(context, CL_MEM_READ_WRITE, height * width * sizeof(char));
    cl::Buffer buffer_debug(context, CL_MEM_READ_WRITE, height * width * 6 * sizeof(char));

    cl::CommandQueue queue(context);  // Erstelle eine Befehlswarteschlange für das OpenCL-Gerät.

    // Flachlege das zweidimensionale Array der aktuellen Generation in ein eindimensionales Array.
    std::vector<char> flat_current(height * width);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            flat_current[i * width + j] = currentGeneration[i][j] ? '0' : '.';  // Lebende Zellen als '0', tote Zellen als '.'.
        }
    }

    cl_int err;
    // Übertrage die flachen Daten der aktuellen Generation auf den GPU-Puffer.
    err = queue.enqueueWriteBuffer(buffer_current, CL_TRUE, 0, height * width * sizeof(char), flat_current.data());
    if (err != CL_SUCCESS) {
        std::cerr << "Error writing buffer_current: " << err << std::endl;
        return;
    }

    // Setze die Kernel-Argumente: Puffer für aktuelle und nächste Generation, Höhe, Breite und Debug-Puffer.
    kernel.setArg(0, buffer_current);
    kernel.setArg(1, buffer_next);
    kernel.setArg(2, height);
    kernel.setArg(3, width);
    kernel.setArg(4, buffer_debug);

    // Definiere den globalen Arbeitsbereich, der der Größe des Gitters entspricht.
    cl::NDRange global(height, width);
    // Füge den Kernel zur Befehlswarteschlange hinzu und führe ihn auf der GPU aus.
    err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    if (err != CL_SUCCESS) {
        std::cerr << "Error enqueueing kernel: " << err << std::endl;
        return;
    }

    queue.finish();  // Warte, bis alle Befehle in der Warteschlange ausgeführt wurden.

    // Lese die Ergebnisse der nächsten Generation vom GPU-Puffer zurück in das flache Array.
    std::vector<char> flat_next(height * width);
    std::vector<char> debug_info(height * width * 6);
    err = queue.enqueueReadBuffer(buffer_next, CL_TRUE, 0, height * width * sizeof(char), flat_next.data());
    if (err != CL_SUCCESS) {
        std::cerr << "Error reading buffer_next: " << err << std::endl;
        return;
    }

    // Lese Debug-Informationen vom GPU-Puffer (falls vorhanden).
    err = queue.enqueueReadBuffer(buffer_debug, CL_TRUE, 0, height * width * 6 * sizeof(char), debug_info.data());
    if (err != CL_SUCCESS) {
        std::cerr << "Error reading buffer_debug: " << err << std::endl;
        return;
    }

    /*
    // Optional: Ausgabe der Debug-Informationen nach dem Lesen vom GPU-Puffer.
    std::cout << "Debug Information (after reading from buffer):\n";
    for (int i = 0; i < height * width; ++i) {
        std::cout << debug_info[i * 6] << debug_info[i * 6 + 1] << debug_info[i * 6 + 2]
                  << debug_info[i * 6 + 3] << debug_info[i * 6 + 4] << debug_info[i * 6 + 5] << "\n";
    }
    */

    // Übertrage die flachen Ergebnisse der nächsten Generation zurück in das zweidimensionale Array.
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            nextGeneration[i][j] = (flat_next[i * width + j] == '0');
        }
    }

    // Tausche die aktuelle und die nächste Generation, um die Berechnung der nächsten Generation zu ermöglichen.
    currentGeneration.swap(nextGeneration);
}

bool Grid::is_stable() {
    // Diese Funktion überprüft, ob das Gitter in einem stabilen Zustand ist,
    // d.h. ob sich die aktuelle Generation nicht von der nächsten Generation unterscheidet.

    // Doppelte Schleifen über jede Zelle des Gitters (x, y).
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // Wenn eine Zelle ihren Zustand ändert, ist das Gitter nicht stabil.
            if (nextGeneration[i][j] != currentGeneration[i][j]) {
                return false;  // Rückgabe false, wenn das Gitter nicht stabil ist.
            }
        }
    }
    return true;  // Rückgabe true, wenn das Gitter stabil ist.
}

void Grid::clearScreen() const {
    // Diese Funktion löscht den Bildschirm, um die Ausgabe des Gitters zu aktualisieren.

    #ifdef _WIN32
        system("cls");  // Verwende den Befehl "cls" für Windows.
    #else
        int result = system("clear");  // Verwende den Befehl "clear" für Unix-basierte Systeme.
        (void)result;  // Unterdrücke die Warnung über den ungenutzten Rückgabewert.
    #endif
}
