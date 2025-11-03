#include "zbior_ary.h"
#include <stdlib.h>
#include <stdio.h>

static long long g_q;
static void sum_equal_mod(zbior_ary A, zbior_ary B, zbior_ary R, long long* it_A, long long* it_B, int* it_R);

static long long min(long long a, long long b) {
    return a < b ? a : b;
}

static long long max(long long a, long long b) {
    return a > b ? a : b;
}

static long long positive_mod(long long a, long long q){
    return (a%q+q)%q;
}

// Funkcja sprawdzajaca czy iterator zbioru A nie wyszedl poza zbior 
// i czy wskazuje na poprawna reszte mod q
static bool good_residue(zbior_ary A, long long it_A, long long residue){
    return (it_A < A.size && positive_mod(A.segs[it_A].start, g_q) == positive_mod(residue, g_q));
}

zbior_ary ciag_arytmetyczny(int a, int q, int b){
    g_q = q;
    zbior_ary R;
    R.segs = (segment_ary*)malloc(sizeof(segment_ary));
    R.segs[0] = (segment_ary){a, b};
    R.size = 1;
    return R;
}

zbior_ary singleton(int a){
    return ciag_arytmetyczny(a, g_q, a);
}

unsigned moc(zbior_ary A){
    unsigned ans = 0;
    for (long long i = 0; i<A.size; i++){
        ans += (unsigned)((A.segs[i].end - A.segs[i].start)/g_q + 1);
    }
    return ans;
}

unsigned ary(zbior_ary A){
    return (unsigned) A.size;
}

// Binary search by znalezc przedzial w ktorym sa tylko segmenty b mod q
// potem normalny binary serach po poczatkach przedzialow
bool nalezy(zbior_ary A, int b){
    long long b_mod_q = positive_mod(b, g_q);
    long long left_q = -1, right_q = -1;

    long long l = 0, r = A.size - 1;
    while (l <= r){
        long long mid = l + (r-l)/2;
        if (positive_mod(A.segs[mid].start, g_q) == b_mod_q){
            left_q = mid;
            r = mid - 1;
        }
        else if (positive_mod(A.segs[mid].start, g_q) > b_mod_q){
            r = mid - 1;
        }
        else {
            l = mid + 1;
        }
    }
    
    l = 0, r = A.size - 1;
    while (l <= r){
        long long mid = l + (r-l)/2;
        if (positive_mod(A.segs[mid].start, g_q) == b_mod_q){
            right_q = mid;
            l = mid + 1;
        }
        else if (positive_mod(A.segs[mid].start, g_q) > b_mod_q){
            r = mid - 1;
        }
        else {
            l = mid + 1;
        }
    }

    // Nie znalezlismy przedzialu ktory jest b mod q
    if (left_q == -1 || right_q == -1){
        return false;
    }

    while (left_q <= right_q){
        long long mid = left_q + (right_q - left_q)/2;
        if (A.segs[mid].start > b){
            right_q = mid - 1;
        }
        else if (A.segs[mid].end < b){
            left_q = mid + 1;
        }
        else {
            return true;
        }
    }

    return false;
}

zbior_ary suma(zbior_ary A, zbior_ary B){
    zbior_ary R;
    R.segs = (segment_ary*)malloc(sizeof(segment_ary)*(size_t)(A.size + B.size));
    R.size = A.size + B.size;
    long long it_A = 0;
    long long it_B = 0;
    long long it_R = 0;

    while (it_A < A.size && it_B < B.size){
        // Iterujemy po każdym ze zbiorow az w koncu znajdziemy dwa segmenty rowne mod q, jesli nie sa to sie nie pokrywaja
        // Przez to mozemy odrazu dodawac segmenty do R i utrzymywac niezmiennik ze segmenty sa rosnace i rozlaczne
        while (it_A < A.size && positive_mod(A.segs[it_A].start, g_q) < positive_mod(B.segs[it_B].start, g_q)){
            R.segs[it_R] = A.segs[it_A];
            it_R++;
            it_A++;
        }

        while (it_B < B.size && positive_mod(B.segs[it_B].start, g_q) < positive_mod(A.segs[it_A].start, g_q)){
            R.segs[it_R] = B.segs[it_B];
            it_R++;
            it_B++;
        }

        // Zostal przypadek gdzie segmenty sa rowne mod q czyli sie pokrywaja
        if (positive_mod(A.segs[it_A].start, g_q) == positive_mod(B.segs[it_B].start, g_q)){
            sum_equal_mod(A, B, R, &it_A, &it_B, &it_R);
        }
    }

    // Dodajemy elementy tego zbioru, do ktorego konca jeszcze nie doszlismy
    for (;it_A < A.size; it_A++){
        R.segs[it_R] = A.segs[it_A];
        it_R++;
    }
    for (;it_B < B.size; it_B++){
        R.segs[it_R] = B.segs[it_B];
        it_R++;
    }

    // Zmniejszamy odpowiednio tablice z segmentami
    R.segs = (segment_ary*)realloc(R.segs, sizeof(segment_ary)*(size_t)it_R);
    R.size = it_R;
    return R;
}

// Funkcja dodająca wszystkie segmenty A i B o podanej reszcie mod q, zapisujac wynik w R
static void sum_equal_mod(zbior_ary A, zbior_ary B, zbior_ary R, long long* start_A, long long* start_B, long long* it_R){
    long long residue = positive_mod(A.segs[*start_A].start, g_q);

    // Iterujemy po zbiorach dopoki sa pozadana reszta mod q
    while (good_residue(A, *start_A, residue) && good_residue(B, *start_B, residue)){
        segment_ary seg_to_add;

        // Wybieramy mniejszy z segmentow
        if (A.segs[*start_A].start < B.segs[*start_B].start){
            seg_to_add = A.segs[*start_A];
            (*start_A)++;
        }
        else {
            seg_to_add = B.segs[*start_B];
            (*start_B)++;
        }
        
        // Laczymy segmenty dopki przestana sie pokrywac
        while (true){
            bool added = false;

            if (good_residue(A, *start_A, residue) && A.segs[*start_A].start <= seg_to_add.end+g_q){
                seg_to_add.end = max(seg_to_add.end, A.segs[*start_A].end);
                (*start_A)++;
                added = true;
            }
            if (good_residue(B, *start_B, residue) && B.segs[*start_B].start <= seg_to_add.end+g_q){
                seg_to_add.end = max(seg_to_add.end, B.segs[*start_B].end);
                (*start_B)++;
                added = true;
            }
            if (!added) break;
        }

        R.segs[*it_R] = seg_to_add;
        (*it_R)++;
    }
    
    // Dodajemy segmenty tego zbioru, do ktorego konca jeszcze nie doszlismy
    for (;good_residue(A, *start_A, residue); (*start_A)++){
        R.segs[*it_R] = A.segs[*start_A];
        (*it_R)++;
    }
    for (;good_residue(B, *start_B, residue); (*start_B)++){
        R.segs[*it_R] = B.segs[*start_B];
        (*it_R)++;
    }
}    

zbior_ary roznica(zbior_ary A, zbior_ary B){
    // Tozsamosc A/B = (A+B)/B pozwala nam dodac na poczatku B do A
    // co powoduje ze kazdy segment B pokrywa sie z dokladnie jednym segmentem A i zawiera sie w nim caly
    A = suma(A, B);

    zbior_ary R;
    R.segs = (segment_ary*)malloc(sizeof(segment_ary)*(size_t)(A.size + B.size));
    R.size = A.size + B.size;

    long long it_B = 0, it_R = 0;
    for (long long it_A = 0; it_A < A.size; it_A++){
        segment_ary current_seg = A.segs[it_A];

        // Odejmujemy segmenty B poki maja one pokrycie z aktualnym segmentem
        while (good_residue(B, it_B, A.segs[it_A].start%g_q) && A.segs[it_A].end >= B.segs[it_B].start){
            segment_ary seg_to_add;
            seg_to_add.start = current_seg.start;
            seg_to_add.end = B.segs[it_B].start - g_q;

            if (seg_to_add.start <= seg_to_add.end){
                R.segs[it_R] = seg_to_add;
                it_R++;
            }

            current_seg.start = B.segs[it_B].end + g_q;
            it_B++;
        }

        // Dodajemy pozostalosc obecnego segmentu A
        if (current_seg.start <= current_seg.end){
            R.segs[it_R] = current_seg;
            it_R++;
        }
    }

    // Odpowiednio zmniejszamy tablice z segmentami
    R.segs = (segment_ary*)realloc(R.segs, sizeof(segment_ary)*(size_t)it_R);
    R.size = it_R;
    return R;
}

zbior_ary iloczyn(zbior_ary A, zbior_ary B){
    // Korzystamy z tozsamosci ze iloczyn A i B jest rowny A/(A/B)
    return roznica(A, roznica(A, B));
}
