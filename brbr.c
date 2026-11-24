#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <windows.h>

#define MAX_N 6
#define MAX_E 10

/* ANSI Renk Kodlari */
#define RESET       L"\033[0m"
#define BOLD        L"\033[1m"
#define CYAN        L"\033[36m"
#define MAGENTA     L"\033[35m"
#define GREEN       L"\033[32m"
#define YELLOW      L"\033[33m"
#define RED         L"\033[31m"
#define CLEAR_SCR   L"\033[H\033[J"

typedef struct {
    int u;
    int v;
} Edge;

/* 1. Gorev: N x N Komsuluk Matrisi olusturma */
void buildAdjacencyMatrix(int n, const Edge edges[], int edgeCount, int adj[MAX_N][MAX_N]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            adj[i][j] = 0;
        }
    }
    for (int e = 0; e < edgeCount; e++) {
        int u = edges[e].u - 1;
        int v = edges[e].v - 1;
        if (u >= 0 && u < n && v >= 0 && v < n) {
            adj[u][v] = 1;
            adj[v][u] = 1;
        }
    }
}

/* 2. Gorev: Dugum dereceleri ve grafin genel derecesi */
void computeDegrees(int n, int adj[MAX_N][MAX_N], int degrees[MAX_N], int *maxDegree, int *totalDegree) {
    *maxDegree = 0;
    *totalDegree = 0;
    for (int i = 0; i < n; i++) {
        int deg = 0;
        for (int j = 0; j < n; j++) {
            deg += adj[i][j];
        }
        degrees[i] = deg;
        *totalDegree += deg;
        if (deg > *maxDegree) {
            *maxDegree = deg;
        }
    }
}

/* 3. Gorev: 1. dugumden (0 / Mavi) baslayarak uzaklik, renk/parite ve bipartite kontrolu */
int checkBipartiteFromNode1(int n, int adj[MAX_N][MAX_N], int color[MAX_N], int dist[MAX_N],
                             int *conflictU, int *conflictV) {
    int queue[MAX_N];
    int front = 0, rear = 0;
    int isBipartite = 1;

    *conflictU = -1;
    *conflictV = -1;

    for (int i = 0; i < n; i++) {
        color[i] = -1;
        dist[i] = -1;
    }

    /* Baslangic dugumu: 1 (indeks 0) -> Renk 0 (Mavi/Cift), Uzaklik 0 */
    color[0] = 0;
    dist[0] = 0;
    queue[rear++] = 0;

    while (front < rear) {
        int u = queue[front++];
        for (int v = 0; v < n; v++) {
            if (adj[u][v] == 1) {
                if (color[v] == -1) {
                    dist[v] = dist[u] + 1;
                    color[v] = dist[v] % 2; /* Cift uzaklik: 0 (Mavi), Tek uzaklik: 1 (Kirmizi) */
                    queue[rear++] = v;
                } else if (color[v] == color[u]) {
                    isBipartite = 0;
                    if (*conflictU == -1) {
                        *conflictU = u + 1;
                        *conflictV = v + 1;
                    }
                }
            }
        }
    }

    return isBipartite;
}

/* Dikdortgen U ve V kumelerini ve animasyonu cizer */
void drawRectanglesAnimation(const wchar_t *stageTitle, const Edge edges[], int edgeCount,
                             int activeIdx, const int degrees[MAX_N], const int color[MAX_N],
                             int isBipartite, int conflictU, int conflictV) {
    const int uSet[3] = {1, 3, 5};
    const int vSet[3] = {2, 4, 6};

    wprintf(CLEAR_SCR);
    wprintf(BOLD YELLOW L"=== %ls ===\n\n" RESET, stageTitle);
    wprintf(BOLD CYAN L"    KUME U (Istemciler)           BAGLANTI ANIMASYONU           KUME V (Sunucular)\n" RESET);
    wprintf(CYAN L"  ┌──────────────────────┐                                   ┌──────────────────────┐\n" RESET);

    for (int r = 0; r < 3; r++) {
        int u = uSet[r];
        int v = vSet[r];
        int cU = color[u - 1];
        int cV = color[v - 1];

        const wchar_t *uCol = (cU == 0) ? CYAN : RED;
        wprintf(CYAN L"  │ " RESET L"%lsDugum %d (p:%d, d:%d)" RESET CYAN L" │" RESET,
                uCol, u, cU, degrees[u - 1]);

        if (r == 1) {
            if (activeIdx >= 0 && activeIdx < edgeCount) {
                Edge e = edges[activeIdx];
                int sameParity = (color[e.u - 1] == color[e.v - 1]);
                if (sameParity) {
                    wprintf(BOLD RED L"   [!] CAKISMA (%d -- %d) [!] " RESET, e.u, e.v);
                } else {
                    wprintf(BOLD YELLOW L"   --> Kenar (%d, %d) <--     " RESET, e.u, e.v);
                }
            } else if (isBipartite) {
                wprintf(BOLD GREEN L"   <=== BIPARTITE (OK) ===>   " RESET);
            } else {
                wprintf(BOLD RED L"   <=== IHLAL (%d,%d) ===>    " RESET, conflictU, conflictV);
            }
        } else {
            wprintf(L"                                   ");
        }

        const wchar_t *vCol = (cV == 1) ? MAGENTA : RED;
        wprintf(MAGENTA L"│ " RESET L"%lsDugum %d (p:%d, d:%d)" RESET MAGENTA L" │\n" RESET,
                vCol, v, cV, degrees[v - 1]);

        if (r < 2) {
            wprintf(CYAN L"  │                      │" RESET L"                                   " MAGENTA L"│                      │\n" RESET);
        }
    }

    wprintf(CYAN L"  └──────────────────────┘                                   └──────────────────────┘\n\n" RESET);
    wprintf(BOLD L"Kenarlar: " RESET);
    for (int e = 0; e < edgeCount; e++) {
        int sameParity = (color[edges[e].u - 1] == color[edges[e].v - 1]);
        if (e == activeIdx) {
            wprintf(sameParity ? BOLD RED L"[!(%d,%d)!] " RESET : BOLD YELLOW L"[ (%d,%d) ] " RESET,
                    edges[e].u, edges[e].v);
        } else {
            wprintf(sameParity ? RED L"(%d,%d) " RESET : GREEN L"(%d,%d) " RESET,
                    edges[e].u, edges[e].v);
        }
    }
    wprintf(L"\n");
    fflush(stdout);
}

void runFullAnalysis(const wchar_t *title, int n, const Edge edges[], int edgeCount) {
    int adj[MAX_N][MAX_N];
    int degrees[MAX_N];
    int maxDegree = 0, totalDegree = 0;
    int color[MAX_N], dist[MAX_N];
    int conflictU = -1, conflictV = -1;

    buildAdjacencyMatrix(n, edges, edgeCount, adj);
    computeDegrees(n, adj, degrees, &maxDegree, &totalDegree);
    int isBipartite = checkBipartiteFromNode1(n, adj, color, dist, &conflictU, &conflictV);

    for (int e = 0; e < edgeCount; e++) {
        drawRectanglesAnimation(title, edges, edgeCount, e, degrees, color,
                                isBipartite, conflictU, conflictV);
        Sleep(400);
    }
    drawRectanglesAnimation(title, edges, edgeCount, -1, degrees, color,
                            isBipartite, conflictU, conflictV);

    wprintf(BOLD CYAN L"\n1. Komsuluk Matrisi (%dx%d):\n" RESET, n, n);
    for (int i = 0; i < n; i++) {
        wprintf(L"[");
        for (int j = 0; j < n; j++) {
            wprintf(adj[i][j] == 1 ? BOLD GREEN L"1" RESET : L"0");
            if (j < n - 1) wprintf(L", ");
        }
        wprintf(L"]\n");
    }

    wprintf(BOLD CYAN L"\n2. Dugum Dereceleri ve Genel Derece:\n" RESET);
    for (int i = 0; i < n; i++) {
        wprintf(L"  Dugum %d -> Derece: %d | Uzaklik(1'den): %d | Parite/Renk: %d (%ls)\n",
                i + 1, degrees[i], dist[i], color[i],
                (color[i] == 0) ? L"0 / Mavi / Cift" : L"1 / Kirmizi / Tek");
    }
    wprintf(L"  Grafin Genel Derecesi (Maksimum Derece): " BOLD YELLOW L"%d\n" RESET, maxDegree);
    wprintf(L"  Dugumlerin Dereceler Toplami: " BOLD YELLOW L"%d\n" RESET, totalDegree);

    wprintf(BOLD CYAN L"\n3. Bipartite Kontrol Sonucu:\n" RESET);
    if (isBipartite) {
        wprintf(BOLD GREEN L"  Sonuc: Graf BIPARTITE (Iki Parcali) DIR.\n" RESET);
    } else {
        wprintf(BOLD RED L"  Sonuc: Graf BIPARTITE DEGILDIR! Cakisan kenar: (%d, %d)\n" RESET,
                conflictU, conflictV);
    }
}

int main(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
    setlocale(LC_ALL, "");

    /* Baslangic grafi (7 kenar) */
    Edge initialEdges[MAX_E] = {
        {1, 2}, {1, 4}, {2, 3}, {3, 4}, {3, 6}, {4, 5}, {5, 6}
    };
    runFullAnalysis(L"GOREV 1-3: BASLANGIC GRAFI (7 KENAR)", MAX_N, initialEdges, 7);

    wprintf(BOLD L"\n4. Gorev (Dinamik Test: (2, 4) kenari ekleme) 2 saniye icinde baslayacak...\n" RESET);
    fflush(stdout);
    Sleep(2000);

    /* 4. Gorev: (2, 4) baglantisi eklendiginde (8 kenar) */
    Edge updatedEdges[MAX_E] = {
        
        {1, 2}, {1, 4}, {2, 3}, {3, 4}, {3, 6}, {4, 5}, {5, 6}, {2, 4}
    };
    runFullAnalysis(L"GOREV 4: DINAMIK TEST - (2, 4) EKLENDI (8 KENAR)", MAX_N, updatedEdges, 8);

    wprintf(BOLD L"\nCikmak icin Enter'a basin...\n" RESET);
    getwchar();
    return 0;
}