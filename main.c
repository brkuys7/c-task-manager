#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>



/*KULLANILAN VERÝ YAPILARI

- Hash Table

- Min Heap

- Queue

*/
//-------------------- STRUCT --------------------

typedef struct Task {
    int id;
    char title[50];
    char description[100];
    int x, y;
    int day, month, year;
    int hour, minute;
    int notified;
} Task;

typedef struct HashNode {
    Task *task;
    struct HashNode *next;
} HashNode;

#define HASH_SIZE 100
HashNode *hashTable[HASH_SIZE];

typedef struct MinHeap {
    Task **tasks;
    int size;
    int capacity;
} MinHeap;

typedef struct QueueNode {
    Task *task;
    struct QueueNode *next;
} QueueNode;

typedef struct Queue {
    QueueNode *front;
    QueueNode *rear;
} Queue;

// -------------------- HASH TABLE --------------------

int hashFunction(int id) {
    return id % HASH_SIZE;
}

void insertHash(Task *task) {
    int index = hashFunction(task->id);
    HashNode *node = malloc(sizeof(HashNode));
    node->task = task;
    node->next = hashTable[index];
    hashTable[index] = node;
}

Task* searchHash(int id) {
    int index = hashFunction(id);
    HashNode *temp = hashTable[index];
    while(temp) {
        if(temp->task->id == id) return temp->task;
        temp = temp->next;
    }
    return NULL;
}

void deleteHash(int id) {
    int index = hashFunction(id);
    HashNode *temp = hashTable[index], *prev = NULL;
    while(temp) {
        if(temp->task->id == id) {
            if(prev) prev->next = temp->next;
            else hashTable[index] = temp->next;
            free(temp->task);
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}

// -------------------- ZAMAN VE MESAFE --------------------

int minutesLeft(Task *t) {
    time_t now = time(NULL);
    struct tm d = {0};
    d.tm_mday = t->day;
    d.tm_mon  = t->month - 1;
    d.tm_year = t->year - 1900;
    d.tm_hour = t->hour;
    d.tm_min  = t->minute;
    d.tm_isdst = -1; // Gün ýþýðýndan yararlanma ayarý
    return (int)difftime(mktime(&d), now) / 60;
}

double distanceCalc(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1-x2, 2) + pow(y1-y2, 2));
}

// -------------------- MIN HEAP --------------------

MinHeap* createHeap(int cap) {
    MinHeap *h = malloc(sizeof(MinHeap));
    h->tasks = malloc(sizeof(Task*) * cap);
    h->size = 0;
    h->capacity = cap;
    return h;
}

void heapifyUp(MinHeap *h, int i, int ux, int uy) {
    if(i == 0) return;
    int p = (i - 1) / 2;
    if(distanceCalc(h->tasks[i]->x, h->tasks[i]->y, ux, uy) <
       distanceCalc(h->tasks[p]->x, h->tasks[p]->y, ux, uy)) {
        Task *t = h->tasks[i];
        h->tasks[i] = h->tasks[p];
        h->tasks[p] = t;
        heapifyUp(h, p, ux, uy);
    }
}

void heapifyDown(MinHeap *h, int i, int ux, int uy) {
    int l = 2*i+1, r = 2*i+2, s = i;
    if(l < h->size && distanceCalc(h->tasks[l]->x, h->tasks[l]->y, ux, uy) <
       distanceCalc(h->tasks[s]->x, h->tasks[s]->y, ux, uy)) s = l;
    if(r < h->size && distanceCalc(h->tasks[r]->x, h->tasks[r]->y, ux, uy) <
       distanceCalc(h->tasks[s]->x, h->tasks[s]->y, ux, uy)) s = r;
    if(s != i) {
        Task *t = h->tasks[i];
        h->tasks[i] = h->tasks[s];
        h->tasks[s] = t;
        heapifyDown(h, s, ux, uy);
    }
}

void insertHeap(MinHeap *h, Task *t, int ux, int uy) {
    if(h->size < h->capacity) {
        h->tasks[h->size] = t;
        heapifyUp(h, h->size, ux, uy);
        h->size++;
    }
}

Task* popHeap(MinHeap *h, int ux, int uy) {
    if(h->size == 0) return NULL;
    Task *t = h->tasks[0];
    h->tasks[0] = h->tasks[--h->size];
    heapifyDown(h, 0, ux, uy);
    return t;
}

void deleteFromHeap(MinHeap *h, int id, int ux, int uy) {
    for(int i=0; i<h->size; i++) {
        if(h->tasks[i]->id == id) {
            h->tasks[i] = h->tasks[--h->size];
            heapifyDown(h, i, ux, uy);
            heapifyUp(h, i, ux, uy);
            return;
        }
    }
}

// -------------------- QUEUE --------------------

Queue* createQueue() {
    Queue *q = malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue *q, Task *t) {
    QueueNode *n = malloc(sizeof(QueueNode));
    n->task = t;
    n->next = NULL;
    if(!q->rear) q->front = q->rear = n;
    else { q->rear->next = n; q->rear = n; }
}

void printQueue(Queue *q) {
    QueueNode *t = q->front;
    if(!t) { printf("\nYaklasan gorev yok.\n"); return; }
    printf("\n--- YAKLASAN GOREVLER ---\n");
    while(t) {
        printf("ID:%d | %s | %d dk kaldi\n", t->task->id, t->task->title, minutesLeft(t->task));
        t = t->next;
    }
}

void checkUpcomingTasks(MinHeap *h, Queue *q) {
    for(int i=0; i<h->size; i++) {
        int m = minutesLeft(h->tasks[i]);
        if(!h->tasks[i]->notified && m > 0 && m <= 1440) {
            enqueue(q, h->tasks[i]);
            h->tasks[i]->notified = 1;
        }
    }
}

// -------------------- MAIN --------------------

void menu() {
    printf("\n--- GOREV TAKIP SISTEMI ---");
    printf("\n1- Gorev Ekle\n2- En Yakin Gorevi Gor\n3- ID ile Ara\n4- En Yakin Gorevi Tamamla");
    printf("\n5- Yaklasan Gorevleri Listele\n6- Konum Degistir\n7- Gorev Sil\n0- Cikis\nSecim: ");
}

int main() {
    int ux=0, uy=0, s;
    for(int i=0; i<HASH_SIZE; i++) hashTable[i] = NULL;
    MinHeap *heap = createHeap(100);
    Queue *notif = createQueue();

    do {
        menu();
        if(scanf("%d", &s) != 1) {
            while(getchar() != '\n');
            continue;
        }
        getchar(); // Buffer temizleme

        switch(s) {
            case 1: {
                Task *t = malloc(sizeof(Task));
                printf("ID: "); scanf("%d", &t->id);
                printf("Baslik: "); scanf(" %[^\n]", t->title);
                printf("Aciklama: "); scanf(" %[^\n]", t->description);
                printf("Konum X Y: "); scanf("%d %d", &t->x, &t->y);
                printf("Tarih (GG AA YYYY SS DD): ");
                scanf("%d %d %d %d %d", &t->day, &t->month, &t->year, &t->hour, &t->minute);
                t->notified = 0;
                insertHash(t);
                insertHeap(heap, t, ux, uy);
                printf("Gorev eklendi.\n");
                break;
            }
            case 2:
                if(heap->size > 0)
                    printf("\nEn Yakin: %s | Mesafe: %.2f\n", heap->tasks[0]->title, distanceCalc(heap->tasks[0]->x, heap->tasks[0]->y, ux, uy));
                else printf("\nGorev listesi bos.\n");
                break;
            case 3: {
                int id; printf("ID: "); scanf("%d", &id);
                Task *res = searchHash(id);
                if(res) printf("Bulundu: %s (%s)\n", res->title, res->description);
                else printf("Bulunamadi.\n");
                break;
            }
            case 4: {
                Task *t = popHeap(heap, ux, uy);
                if(t) { printf("Tamamlandi ve Silindi: %s\n", t->title); deleteHash(t->id); }
                else printf("Gorev yok.\n");
                break;
            }
            case 5:
                checkUpcomingTasks(heap, notif);
                printQueue(notif);
                break;
            case 6:
                printf("Yeni Konum X Y: "); scanf("%d %d", &ux, &uy);
                for(int i = (heap->size/2)-1; i >= 0; i--) heapifyDown(heap, i, ux, uy);
                printf("Konum guncellendi. Mesafeler yeniden hesaplandi.\n");
                break;
            case 7: {
                int id; printf("Silinecek ID: "); scanf("%d", &id);
                if(searchHash(id)) {
                    deleteFromHeap(heap, id, ux, uy);
                    deleteHash(id);
                    printf("Silindi.\n");
                } else printf("ID bulunamadi.\n");
                break;
            }
        }
    } while(s != 0);

    return 0;
}
