#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

enum { INS, REM, BUS };

long long agora() {
    return chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now().time_since_epoch()).count();
}

template <class N, class F>
void tracar(N* x, N* nulo, const string& pre, bool dir, F rotulo) {
    if (x == nulo) return;
    tracar(x->dir, nulo, pre + (dir ? "    " : "|   "), true, rotulo);
    printf("%s%s%s\n", pre.c_str(), dir ? "/-- " : "\\-- ", rotulo(x).c_str());
    tracar(x->esq, nulo, pre + (dir ? "|   " : "    "), false, rotulo);
}

template <class N, class F>
void desenhar(N* raiz, N* nulo, F rotulo) {
    if (raiz == nulo) {
        puts("Arvore vazia");
        return;
    }
    tracar(raiz->dir, nulo, "", true, rotulo);
    printf("%s\n", rotulo(raiz).c_str());
    tracar(raiz->esq, nulo, "", false, rotulo);
}

struct Arvore {
    long long rot = 0, rec = 0, tempo[3] = {0, 0, 0}, cont[3] = {0, 0, 0};
    size_t n = 0, pico = 0;
    bool colorida = false;

    virtual ~Arvore() {}
    virtual const char* nome() = 0;
    virtual const char* propriedades() = 0;
    virtual size_t tamNo() = 0;
    virtual bool ins(int k) = 0;
    virtual bool rem(int k) = 0;
    virtual bool bus(int k) = 0;
    virtual void coletar(vector<int>& v) = 0;
    virtual int altura() = 0;
    virtual bool balanceada() = 0;
    virtual void imprimir() = 0;

    void alocou() {
        if (++n > pico) pico = n;
    }
    void liberou() { n--; }

    bool inserir(int k) {
        long long t = agora();
        bool r = ins(k);
        tempo[INS] += agora() - t;
        cont[INS]++;
        return r;
    }
    bool remover(int k) {
        long long t = agora();
        bool r = rem(k);
        tempo[REM] += agora() - t;
        cont[REM]++;
        return r;
    }
    bool buscar(int k) {
        long long t = agora();
        bool r = bus(k);
        tempo[BUS] += agora() - t;
        cont[BUS]++;
        return r;
    }
    long long lote(int op, const vector<int>& v) {
        long long ok = 0, t = agora();
        for (int k : v) ok += (op == INS ? ins(k) : op == REM ? rem(k) : bus(k));
        tempo[op] += agora() - t;
        cont[op] += v.size();
        return ok;
    }

    bool validar() {
        vector<int> v;
        coletar(v);
        for (size_t i = 1; i < v.size(); i++)
            if (v[i - 1] >= v[i]) return false;
        return v.size() == n && balanceada();
    }

    void relatorio() {
        const char* ops[3] = {"Insercao", "Remocao", "Busca"};
        printf("\n===== Relatorio: %s =====\n", nome());
        printf("Elementos: %lld | Altura (em nos): %d\n", (long long)n, altura());
        printf("Rotacoes: %lld", rot);
        if (colorida) printf(" | Recoloracoes: %lld", rec);
        puts("\nTempo (ns):");
        for (int i = 0; i < 3; i++)
            printf("  %-9s operacoes=%-8lld total=%-12lld media=%.1f\n", ops[i], cont[i], tempo[i],
                   cont[i] ? (double)tempo[i] / cont[i] : 0.0);
        printf("Memoria dos nos: %lld B/no | em uso: %lld B | pico: %lld B\n", (long long)tamNo(),
               (long long)(n * tamNo()), (long long)(pico * tamNo()));
    }
};

struct NoAVL {
    int chave, h;
    NoAVL *esq, *dir;
};

struct AVL : Arvore {
    NoAVL* raiz = nullptr;

    ~AVL() { apagar(raiz); }
    const char* nome() override { return "AVL"; }
    const char* propriedades() override { return "ordem BST, alturas corretas e fator de balanceamento em {-1, 0, 1}"; }
    size_t tamNo() override { return sizeof(NoAVL); }

    static int alt(NoAVL* x) { return x ? x->h : 0; }
    static int fb(NoAVL* x) { return alt(x->esq) - alt(x->dir); }
    static void atualiza(NoAVL* x) { x->h = 1 + max(alt(x->esq), alt(x->dir)); }

    void apagar(NoAVL* x) {
        if (!x) return;
        apagar(x->esq);
        apagar(x->dir);
        delete x;
    }

    NoAVL* rotD(NoAVL* y) {
        NoAVL* x = y->esq;
        y->esq = x->dir;
        x->dir = y;
        atualiza(y);
        atualiza(x);
        rot++;
        return x;
    }
    NoAVL* rotE(NoAVL* x) {
        NoAVL* y = x->dir;
        x->dir = y->esq;
        y->esq = x;
        atualiza(x);
        atualiza(y);
        rot++;
        return y;
    }
    NoAVL* balancear(NoAVL* x) {
        atualiza(x);
        int b = fb(x);
        if (b > 1) {
            if (fb(x->esq) < 0) x->esq = rotE(x->esq);
            return rotD(x);
        }
        if (b < -1) {
            if (fb(x->dir) > 0) x->dir = rotD(x->dir);
            return rotE(x);
        }
        return x;
    }

    NoAVL* insere(NoAVL* x, int k, bool& ok) {
        if (!x) {
            ok = true;
            alocou();
            return new NoAVL{k, 1, nullptr, nullptr};
        }
        if (k < x->chave) x->esq = insere(x->esq, k, ok);
        else if (k > x->chave) x->dir = insere(x->dir, k, ok);
        else {
            ok = false;
            return x;
        }
        return balancear(x);
    }

    NoAVL* retira(NoAVL* x, int k, bool& ok) {
        if (!x) {
            ok = false;
            return nullptr;
        }
        if (k < x->chave) x->esq = retira(x->esq, k, ok);
        else if (k > x->chave) x->dir = retira(x->dir, k, ok);
        else {
            ok = true;
            if (!x->esq || !x->dir) {
                NoAVL* filho = x->esq ? x->esq : x->dir;
                delete x;
                liberou();
                return filho;
            }
            NoAVL* s = x->dir;
            while (s->esq) s = s->esq;
            x->chave = s->chave;
            x->dir = retira(x->dir, s->chave, ok);
        }
        return balancear(x);
    }

    bool ins(int k) override {
        bool ok;
        raiz = insere(raiz, k, ok);
        return ok;
    }
    bool rem(int k) override {
        bool ok;
        raiz = retira(raiz, k, ok);
        return ok;
    }
    bool bus(int k) override {
        NoAVL* x = raiz;
        while (x && x->chave != k) x = k < x->chave ? x->esq : x->dir;
        return x != nullptr;
    }

    void em(NoAVL* x, vector<int>& v) {
        if (!x) return;
        em(x->esq, v);
        v.push_back(x->chave);
        em(x->dir, v);
    }
    void coletar(vector<int>& v) override { em(raiz, v); }
    int altura() override { return alt(raiz); }

    int confere(NoAVL* x, bool& ok) {
        if (!x) return 0;
        int a = confere(x->esq, ok), b = confere(x->dir, ok);
        if (abs(a - b) > 1 || x->h != 1 + max(a, b)) ok = false;
        return 1 + max(a, b);
    }
    bool balanceada() override {
        bool ok = true;
        confere(raiz, ok);
        return ok;
    }

    void imprimir() override {
        desenhar<NoAVL>(raiz, nullptr, [](NoAVL* x) { return to_string(x->chave) + " (fb=" + to_string(fb(x)) + ")"; });
    }
};

struct NoRN {
    int chave;
    bool vermelho;
    NoRN *esq, *dir, *pai;
};

struct RN : Arvore {
    NoRN* nil;
    NoRN* raiz;

    RN() {
        nil = new NoRN{0, false, nullptr, nullptr, nullptr};
        nil->esq = nil->dir = nil->pai = nil;
        raiz = nil;
        colorida = true;
    }
    ~RN() {
        apagar(raiz);
        delete nil;
    }
    const char* nome() override { return "Rubro-Negra"; }
    const char* propriedades() override {
        return "ordem BST, raiz preta, sem vermelhos consecutivos e mesma altura negra em todos os caminhos";
    }
    size_t tamNo() override { return sizeof(NoRN); }

    void apagar(NoRN* x) {
        if (x == nil) return;
        apagar(x->esq);
        apagar(x->dir);
        delete x;
    }
    void pinta(NoRN* x, bool vermelho) {
        if (x->vermelho != vermelho) {
            x->vermelho = vermelho;
            rec++;
        }
    }

    void rotE(NoRN* x) {
        NoRN* y = x->dir;
        x->dir = y->esq;
        if (y->esq != nil) y->esq->pai = x;
        y->pai = x->pai;
        if (x->pai == nil) raiz = y;
        else if (x == x->pai->esq) x->pai->esq = y;
        else x->pai->dir = y;
        y->esq = x;
        x->pai = y;
        rot++;
    }
    void rotD(NoRN* x) {
        NoRN* y = x->esq;
        x->esq = y->dir;
        if (y->dir != nil) y->dir->pai = x;
        y->pai = x->pai;
        if (x->pai == nil) raiz = y;
        else if (x == x->pai->dir) x->pai->dir = y;
        else x->pai->esq = y;
        y->dir = x;
        x->pai = y;
        rot++;
    }
    void transp(NoRN* u, NoRN* v) {
        if (u->pai == nil) raiz = v;
        else if (u == u->pai->esq) u->pai->esq = v;
        else u->pai->dir = v;
        v->pai = u->pai;
    }

    void consertaIns(NoRN* z) {
        while (z->pai->vermelho) {
            NoRN* avo = z->pai->pai;
            bool paiEsq = z->pai == avo->esq;
            NoRN* tio = paiEsq ? avo->dir : avo->esq;
            if (tio->vermelho) {
                pinta(z->pai, false);
                pinta(tio, false);
                pinta(avo, true);
                z = avo;
            } else {
                if (paiEsq ? z == z->pai->dir : z == z->pai->esq) {
                    z = z->pai;
                    paiEsq ? rotE(z) : rotD(z);
                }
                pinta(z->pai, false);
                pinta(avo, true);
                paiEsq ? rotD(avo) : rotE(avo);
            }
        }
        pinta(raiz, false);
    }

    void consertaRem(NoRN* x) {
        while (x != raiz && !x->vermelho) {
            bool xEsq = x == x->pai->esq;
            NoRN* w = xEsq ? x->pai->dir : x->pai->esq;
            if (w->vermelho) {
                pinta(w, false);
                pinta(x->pai, true);
                xEsq ? rotE(x->pai) : rotD(x->pai);
                w = xEsq ? x->pai->dir : x->pai->esq;
            }
            NoRN* perto = xEsq ? w->esq : w->dir;
            NoRN* longe = xEsq ? w->dir : w->esq;
            if (!perto->vermelho && !longe->vermelho) {
                pinta(w, true);
                x = x->pai;
            } else {
                if (!longe->vermelho) {
                    pinta(perto, false);
                    pinta(w, true);
                    xEsq ? rotD(w) : rotE(w);
                    w = xEsq ? x->pai->dir : x->pai->esq;
                }
                pinta(w, x->pai->vermelho);
                pinta(x->pai, false);
                pinta(xEsq ? w->dir : w->esq, false);
                xEsq ? rotE(x->pai) : rotD(x->pai);
                x = raiz;
            }
        }
        pinta(x, false);
    }

    NoRN* achar(int k) {
        NoRN* x = raiz;
        while (x != nil && x->chave != k) x = k < x->chave ? x->esq : x->dir;
        return x;
    }

    bool ins(int k) override {
        NoRN* pai = nil;
        NoRN* x = raiz;
        while (x != nil) {
            if (k == x->chave) return false;
            pai = x;
            x = k < x->chave ? x->esq : x->dir;
        }
        NoRN* z = new NoRN{k, true, nil, nil, pai};
        alocou();
        if (pai == nil) raiz = z;
        else if (k < pai->chave) pai->esq = z;
        else pai->dir = z;
        consertaIns(z);
        return true;
    }

    bool rem(int k) override {
        NoRN* z = achar(k);
        if (z == nil) return false;
        NoRN* y = z;
        NoRN* x;
        bool corOriginal = y->vermelho;
        if (z->esq == nil) {
            x = z->dir;
            transp(z, z->dir);
        } else if (z->dir == nil) {
            x = z->esq;
            transp(z, z->esq);
        } else {
            y = z->dir;
            while (y->esq != nil) y = y->esq;
            corOriginal = y->vermelho;
            x = y->dir;
            if (y->pai == z) {
                x->pai = y;
            } else {
                transp(y, y->dir);
                y->dir = z->dir;
                y->dir->pai = y;
            }
            transp(z, y);
            y->esq = z->esq;
            y->esq->pai = y;
            y->vermelho = z->vermelho;
        }
        delete z;
        liberou();
        if (!corOriginal) consertaRem(x);
        nil->pai = nil;
        return true;
    }

    bool bus(int k) override { return achar(k) != nil; }

    void em(NoRN* x, vector<int>& v) {
        if (x == nil) return;
        em(x->esq, v);
        v.push_back(x->chave);
        em(x->dir, v);
    }
    void coletar(vector<int>& v) override { em(raiz, v); }

    int alt(NoRN* x) { return x == nil ? 0 : 1 + max(alt(x->esq), alt(x->dir)); }
    int altura() override { return alt(raiz); }

    int alturaNegra(NoRN* x, bool& ok) {
        if (x == nil) return 1;
        if (x->vermelho && (x->esq->vermelho || x->dir->vermelho)) ok = false;
        if ((x->esq != nil && x->esq->pai != x) || (x->dir != nil && x->dir->pai != x)) ok = false;
        int a = alturaNegra(x->esq, ok), b = alturaNegra(x->dir, ok);
        if (a != b) ok = false;
        return a + (x->vermelho ? 0 : 1);
    }
    bool balanceada() override {
        bool ok = !raiz->vermelho && raiz->pai == nil;
        alturaNegra(raiz, ok);
        return ok;
    }

    void imprimir() override {
        desenhar<NoRN>(raiz, nil, [](NoRN* x) { return to_string(x->chave) + (x->vermelho ? " [VERMELHO]" : " [PRETO]"); });
    }
};

unique_ptr<Arvore> criar(int tipo) {
    if (tipo == 0) return unique_ptr<Arvore>(new AVL);
    return unique_ptr<Arvore>(new RN);
}

string linha() {
    string s;
    if (!getline(cin, s)) exit(0);
    return s;
}

int lerNum(const char* msg = "Opcao: ") {
    printf("%s", msg);
    fflush(stdout);
    istringstream is(linha());
    int x;
    return (is >> x) ? x : -1;
}

vector<int> lerLista(const char* msg) {
    printf("%s", msg);
    fflush(stdout);
    istringstream is(linha());
    vector<int> v;
    int x;
    while (is >> x) v.push_back(x);
    if (!is.eof()) puts("Aviso: entrada inválida ignorada a partir do primeiro valor incorreto.");
    if (v.empty()) puts("Nenhum valor válido informado.");
    return v;
}

void menuArvore(unique_ptr<Arvore>& a, int tipo) {
    while (true) {
        printf("\n--- Árvore %s ---\n", a->nome());
        puts("1) Inserir");
        puts("2) Remover");
        puts("3) Buscar");
        puts("4) Percurso em ordem");
        puts("5) Imprimir árvore");
        puts("6) Verificar propriedades");
        puts("7) Relatório (tempo, memória, rotações)");
        puts("8) Esvaziar árvore e zerar contadores");
        puts("0) Voltar");
        int op = lerNum();
        if (op == 0) return;
        if (op == 1) {
            for (int k : lerLista("Valores a inserir (separados por espaço): "))
                printf("%d: %s\n", k, a->inserir(k) ? "inserido" : "já existe, inserção ignorada");
        } else if (op == 2) {
            for (int k : lerLista("Valores a remover (separados por espaço): "))
                printf("%d: %s\n", k, a->remover(k) ? "removido" : "não encontrado, nada removido");
        } else if (op == 3) {
            for (int k : lerLista("Valores a buscar (separados por espaço): "))
                printf("%d: %s\n", k, a->buscar(k) ? "encontrado" : "não encontrado");
        } else if (op == 4) {
            vector<int> v;
            a->coletar(v);
            if (v.empty()) {
                puts("Árvore vazia");
            } else {
                printf("Em ordem:");
                for (int k : v) printf(" %d", k);
                puts("");
            }
        } else if (op == 5) {
            a->imprimir();
        } else if (op == 6) {
            printf("Propriedades (%s): %s\n", a->propriedades(), a->validar() ? "OK" : "VIOLADAS");
        } else if (op == 7) {
            a->relatorio();
        } else if (op == 8) {
            a = criar(tipo);
            puts("Árvore esvaziada.");
        } else {
            puts("Opção inválida.");
        }
    }
}

void comparar() {
    int N = lerNum("Quantidade de elementos N (2 a 2000000): ");
    if (N < 2 || N > 2000000) {
        puts("Valor inválido.");
        return;
    }
    vector<int> cres(N), decr(N), alea(N), ausentes(N);
    for (int i = 0; i < N; i++) {
        cres[i] = i + 1;
        decr[i] = N - i;
        alea[i] = i + 1;
        ausentes[i] = N + i + 1;
    }
    mt19937 gen(42);
    shuffle(alea.begin(), alea.end(), gen);
    const char* nomes[3] = {"crescente", "decrescente", "aleatorio"};
    vector<int>* conj[3] = {&cres, &decr, &alea};
    int hmin = 0;
    while ((1LL << hmin) < N + 1LL) hmin++;
    printf("\nN = %d | altura minima possivel = %d (alturas em nos)\n", N, hmin);
    puts("Sequencia: inserir N | buscar N existentes + N inexistentes | remover os N/2 primeiros do conjunto");
    printf("%-11s %-11s %7s %7s %9s %9s %8s %8s %8s %8s  %s\n", "Conjunto", "Árvore", "Alt.ins", "Alt.fin",
           "Rotações", "Recolor.", "Ins(ms)", "Bus(ms)", "Rem(ms)", "Mem(KB)", "válida");
    for (int c = 0; c < 3; c++)
        for (int tipo = 0; tipo < 2; tipo++) {
            unique_ptr<Arvore> t = criar(tipo);
            long long ni = t->lote(INS, *conj[c]);
            int h1 = t->altura();
            bool ok = t->validar();
            long long nb1 = t->lote(BUS, *conj[c]), nb2 = t->lote(BUS, ausentes);
            long long nr = t->lote(REM, vector<int>(conj[c]->begin(), conj[c]->begin() + N / 2));
            ok = ok && t->validar() && ni == N && nb1 == N && nb2 == 0 && nr == N / 2;
            string rc = t->colorida ? to_string(t->rec) : "-";
            printf("%-11s %-11s %7d %7d %9lld %9s %8.3f %8.3f %8.3f %8.1f  %s\n", nomes[c], t->nome(), h1,
                   t->altura(), t->rot, rc.c_str(), t->tempo[INS] / 1e6, t->tempo[BUS] / 1e6, t->tempo[REM] / 1e6,
                   t->pico * t->tamNo() / 1024.0, ok ? "OK" : "ERRO");
        }
}

int main() {
    unique_ptr<Arvore> arv[2] = {criar(0), criar(1)};
    while (true) {
        puts("\n=== Árvores Balanceadas ===");
        puts("1) Árvore AVL");
        puts("2) Árvore Rubro-Negra");
        puts("3) Comparar as duas árvores");
        puts("0) Sair");
        int op = lerNum();
        if (op == 0) return 0;
        if (op == 1 || op == 2) menuArvore(arv[op - 1], op - 1);
        else if (op == 3) comparar();
        else puts("Opção inválida.");
    }
}
