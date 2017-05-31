//
//  hull.c
//
//  Created by Rachael Schwartz on 1/20/17.
//
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct point {
    float x, y;
    struct point *prev, *next;
} point;

void AddToList(point *head, float pI[2]) {
    point *curr = head;
    while (curr->next != NULL)
        curr = curr->next;
    curr->next = malloc(sizeof(point));
    curr->next->x = pI[0]; curr->next->y = pI[1]; curr->next->prev = curr; curr->next->next = NULL;
}

void WriteToFile(FILE *fps, int *n, float ptset[][2]){
    fprintf(fps,"%i\n", *n);
    for(int i=0; i<*n; i++)
        fprintf(fps, "%f %f\n", ptset[i][0], ptset[i][1]);
}

void WriteEpsHeader(FILE *eps){
    fprintf(eps, "%%!PS-Adobe-3.0 EPSF-3.0\n %%BoundingBox: 0 0 1000 1000\n\n gsave\n");
    fprintf(eps, "1 setlinejoin\n 1 setlinecap\n 0 setgray\n 0.5 setlinewidth\n\n");
    fprintf(eps, "10 10 scale\n 30 30 translate\n\n");
}

void DrawPoint(FILE *eps, float x, float y){
    fprintf(eps, "newpath\n %f %f moveto\n %f %f lineto\n closepath\n stroke\n", x, y, x, y);
}

void DrawEdge(FILE *eps, float x1, float y1, float x2, float y2) {
    fprintf(eps, "newpath\n 0.09 setlinewidth\n %f %f moveto\n %f %f lineto\n closepath\n stroke\n", x1, y1, x2, y2);
}

void MakeGenPos(int *n, float ptset[][2]){
    srand (time(NULL));
    for (int i=0; i<*n; i++) {  //create points
        switch (i % 4) {
            case 0: ptset[i][0] = rand() % 30; ptset[i][1] = rand() % 30; break;
            case 1: ptset[i][0] = -(rand() % 30); ptset[i][1] = rand() % 30; break;
            case 2: ptset[i][0] = -(rand() % 30); ptset[i][1] = -(rand() % 30); break;
            case 3: ptset[i][0] = rand() % 30; ptset[i][1] = -(rand() % 30); break;
            default: fprintf(stderr, "Invalid entry; point set could not be created.");   }}
    for (int i=0; i<*n; i++){ int j=0;    //general position (x-distinct)
        while (j<*n) {
            if ((i != j) && (ptset[i][0] == ptset[j][0])) {
                ptset[i][0] = ptset[i][0] + 0.0015;
                j=0; continue; } j++;    }}
    for (int i=(*n-1); i>=0; i--){
        for (int j=0; j<i; j++){    //sort by x-value (least to greatest)
            if (ptset[j][0] > ptset[j+1][0]) {
                float tmp1 = ptset[j][0], tmp2 = ptset[j][1];
                ptset[j][0] = ptset[j+1][0]; ptset[j][1] = ptset[j+1][1];
                ptset[j+1][0] = tmp1; ptset[j+1][1] = tmp2; }   }}
}

int SortUpperLower(float p0[2], float pI[2], float *m) {
    if (((*m * (pI[0] - p0[0])) + p0[1]) < pI[1]) {
    return 1; } else { return 0; }
}

int ListSize(point *head){
    int size=1; point *h = head;
    while (h->next != NULL) { h = h->next; size++; }
    return size;
}

int ConvCondition(point *head, float *m, int k){
    point *h = head; switch(k) {
        case 0: return (h->y >= *m); break;
        case 1: return (h->y < *m); break;
        default: fprintf(stderr, "Sorry, there was a problem with the program."); }
    return 0;
}

int CheckConvexity(point *head, int *k){
    point *h = head;
    if (h->next->next == NULL) { return 1; }
    while (h->next != NULL)
        h = h->next;
    while (h->prev->prev != NULL) {
        float m = ((((h->y - h->prev->prev->y)/(h->x - h->prev->prev->x)) * (h->prev->x - h->x)) + h->y);
        if (ConvCondition(h->prev, &m, *k)) {
            point *tmp = NULL; tmp = malloc(sizeof(point)); tmp = h->prev;
            h->prev = tmp->prev; tmp->prev->next = h;
            free(tmp); return 0; }
        h = h->prev; }
    return 1;
}

void MakeHull(FILE *eps, point *curr, int k){
    int conv;
    if (ListSize(curr) < 3) { if (ListSize(curr) < 2) { return; }
        DrawEdge(eps, curr->x, curr->y, curr->next->x, curr->next->y); return; }
    point *hull = NULL; hull = malloc(sizeof(point));
    hull->x = curr->x; hull->y = curr->y; hull->prev = NULL; hull->next = NULL;
    printf("Curr: %i\n", ListSize(curr));
    while (curr->next != NULL) {
        float P[] = {curr->next->x, curr->next->y};
        if (ListSize(hull) >= 3) {
            conv=0; while (!conv)
                conv = CheckConvexity(hull, &k); }
        AddToList(hull, P);
        curr = curr->next; }
    conv=0; while (!conv)
        conv = CheckConvexity(hull, &k);
    while (curr->prev != NULL)
        curr = curr->prev;
    while (hull->prev != NULL)
        hull = curr->next;
    *curr = *hull;
    printf("Hull: %i\n", ListSize(hull));
    return;
}

int main(){
    int n;
    printf("Enter number of points: ");
    scanf("%i", &n);
    if (n<3) { fprintf(stderr, "Please pick a number greater than 3: ");
        scanf("%i", &n); }
    float set2 [n][2];
    MakeGenPos(&n, set2);
    FILE *fps, *eps_file;
    fps = fopen("pointset.txt", "w");
    if (fps == NULL) { printf("File does not exist.\n");
        return 0; }
    WriteToFile(fps, &n, set2);
    fclose(fps);
    struct point headUP = { set2[0][0], set2[0][1], NULL, NULL };
    struct point headLOW = { set2[0][0], set2[0][1], NULL, NULL };
    float m = ((set2[n-1][1] - set2[0][1])/(set2[n-1][0] - set2[0][0]));
    for (int i=1; i<n; i++){
        if (i<n-1) {
            if (SortUpperLower(set2[0], set2[i], &m))
                AddToList(&headUP, set2[i]);
            else
                AddToList(&headLOW, set2[i]);
        } else { AddToList(&headUP, set2[i]); AddToList(&headLOW, set2[i]); }}
    fclose(fps);
    if ((headUP.next == NULL) || (headLOW.next == NULL)) {
        fprintf(stderr, "Invalid upper or lower hull size. Please try another entry: ");
        scanf("%i", &n); } //not working
    for (point *curr = &headUP; curr != NULL; curr = curr->next)
        printf("%f %f\n", curr->x, curr->y);
    printf("\n");
    for (point *curr = &headLOW; curr != NULL; curr = curr->next)
        printf("%f %f\n", curr->x, curr->y);
    printf("\n");
    eps_file = fopen("drawhull2D.eps", "w");
    if (eps_file == NULL) { printf("Could not open drawhull2D.eps.\n");
        return 0; }
    WriteEpsHeader(eps_file);
    for (point *curr = &headUP; curr != NULL; curr = curr->next)
        DrawPoint(eps_file, curr->x, curr->y);
    printf("\n");
    for (point *curr = &headLOW; curr != NULL; curr = curr->next)
        DrawPoint(eps_file, curr->x, curr->y);
    MakeHull(eps_file, &headUP, 1); MakeHull(eps_file, &headLOW, 0);
    for (point *h = &headUP; h->next != NULL; h = h->next)
        DrawEdge(eps_file, h->x, h->y, h->next->x, h->next->y);
    for (point *h = &headLOW; h->next != NULL; h = h->next)
        DrawEdge(eps_file, h->x, h->y, h->next->x, h->next->y);
    fprintf(eps_file, "grestore\n showpage\n");
    fclose(eps_file);
    return 0;
}





