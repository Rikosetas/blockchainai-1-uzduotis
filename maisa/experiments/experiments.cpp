#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include <cstdio>
#include <array>
#include <filesystem>
#include "../maisa/myhash.h"

using std::string;
using std::vector;
namespace fs = std::filesystem;

struct SM {
    uint64_t s;
    explicit SM(uint64_t seed) : s(seed) {}
    uint64_t next() {
        uint64_t z = (s += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }
    uint32_t below(uint32_t m) { return (uint32_t)(next() % m); }
};

const int ALPHA_LO = 33, ALPHA_HI = 126, ALPHA_N = ALPHA_HI - ALPHA_LO + 1;

string random_string(SM& rng, int len) {
    string s(len, ' ');
    for (int i = 0; i < len; ++i) s[i] = (char)(ALPHA_LO + rng.below(ALPHA_N));
    return s;
}

int popcount8(uint8_t x) { int c = 0; while (x) { c += x & 1; x >>= 1; } return c; }
int hexval(char c) { return (c >= '0' && c <= '9') ? c - '0' : c - 'a' + 10; }

void hex_to_bytes(const string& h, uint8_t out[32]) {
    for (int i = 0; i < 32; ++i) out[i] = (uint8_t)((hexval(h[2*i]) << 4) | hexval(h[2*i+1]));
}
int bit_diff(const string& a, const string& b) {
    uint8_t x[32], y[32]; hex_to_bytes(a, x); hex_to_bytes(b, y);
    int d = 0; for (int i = 0; i < 32; ++i) d += popcount8(x[i] ^ y[i]); return d;
}
int hex_diff(const string& a, const string& b) {
    int d = 0; for (int i = 0; i < 64; ++i) if (a[i] != b[i]) ++d; return d;
}

string HV(const string& s, int v) { return myhash::hash(s, v); }

void exp1_inputs() {
    fs::create_directories("data/inputs");
    struct Case { string name; string bytes; string note; };
    vector<Case> cases;
    cases.push_back({"empty", "", "0 baitu, tuscia"});
    cases.push_back({"one_a", "a", "1 baitas"});
    cases.push_back({"one_b", "b", "1 baitas"});
    SM rng(12345);
    string r = random_string(rng, 2000);
    cases.push_back({"rand2000", r, "2000 atsitiktiniu ASCII"});
    { string x=r; x[0]=(x[0]=='A'?'B':'A'); cases.push_back({"rand2000_first", x, "pakeistas 1-as baitas"}); }
    { string x=r; x[1000]=(x[1000]=='A'?'B':'A'); cases.push_back({"rand2000_mid", x, "pakeistas vidurio baitas"}); }
    { string x=r; x[1999]=(x[1999]=='A'?'B':'A'); cases.push_back({"rand2000_last", x, "pakeistas paskutinis baitas"}); }
    cases.push_back({"repeat32a", string(32,'a'), "pasikartojantys simboliai"});
    cases.push_back({"perm_abcdef", "abcdef", "perstatymas A"});
    cases.push_back({"perm_fedcba", "fedcba", "perstatymas B"});
    cases.push_back({"lead_space", " hello", "tarpas pradzioje"});
    cases.push_back({"trail_space", "hello ", "tarpas pabaigoje"});
    cases.push_back({"no_newline", "hello", "be naujos eilutes"});
    cases.push_back({"with_newline", "hello\n", "su nauja eilute"});
    cases.push_back({"utf8", string("\x41\xC4\x8D\x69\xC5\xAB\xE2\x82\xAC"), "UTF-8: 5 simboliai, 9 baitai"});

    std::ofstream csv("results/exp1_inputs.csv");
    csv << "name,bytes,note,v1,v2\n";
    for (auto& c : cases) {
        std::ofstream f("data/inputs/" + c.name + ".bin", std::ios::binary);
        f.write(c.bytes.data(), (std::streamsize)c.bytes.size());
        f.close();
        csv << c.name << "," << c.bytes.size() << ",\"" << c.note << "\","
            << HV(c.bytes,1) << "," << HV(c.bytes,2) << "\n";
    }
    std::cout << "[1] Ivestys: " << cases.size() << " atveju -> results/exp1_inputs.csv, data/inputs/\n";
}

void exp2_format() {
    SM rng(777);
    int checked = 0, ok = 0, leading_zero = 0;
    std::ofstream csv("results/exp2_format.csv");
    csv << "sample,len_ok,hex_ok,leading_zero,file_eq_mem\n";
    for (int i = 0; i < 20; ++i) {
        string in = random_string(rng, 10 + rng.below(50));
        string h = HV(in, 2);
        bool len_ok = (h.size() == 64);
        bool hex_ok = true;
        for (char c : h) if (!((c>='0'&&c<='9')||(c>='a'&&c<='f'))) hex_ok = false;
        bool lz = (h[0] == '0');
        { std::ofstream f("results/_tmp.bin", std::ios::binary); f.write(in.data(),(std::streamsize)in.size()); }
        std::ifstream f("results/_tmp.bin", std::ios::binary);
        string fromfile((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        bool feq = (HV(fromfile,2) == h);
        checked++; if (len_ok && hex_ok && feq) ok++; if (lz) leading_zero++;
        csv << i << "," << len_ok << "," << hex_ok << "," << lz << "," << feq << "\n";
    }
    fs::remove("results/_tmp.bin");
    std::cout << "[2] Formatas: " << ok << "/" << checked
              << " praejo (64 hex, failas==atmintis); pradiniu nuliu atveju: " << leading_zero << "\n";
}

void exp3_determinism() {
    string A = "Lietuva", B = "Vilnius";
    string hA = HV(A,2);
    bool repeat_ok = true;
    for (int i = 0; i < 1000; ++i) if (HV(A,2) != hA) repeat_ok = false;
    string s1 = HV(A,2), s2 = HV(B,2), s3 = HV(A,2);
    bool seq_ok = (s1 == s3) && (s1 != s2);
    std::ofstream csv("results/exp3_determinism.csv");
    csv << "check,result\n";
    csv << "repeat_1000x_same," << (repeat_ok?"PASS":"FAIL") << "\n";
    csv << "sequence_A_B_A," << (seq_ok?"PASS":"FAIL") << "\n";
    csv << "hash_A," << hA << "\n";
    csv << "hash_B," << s2 << "\n";
    std::cout << "[3] Determinizmas: kartojimas=" << (repeat_ok?"PASS":"FAIL")
              << ", seka A,B,A=" << (seq_ok?"PASS":"FAIL") << "\n";
}

void exp4_speed() {
    std::ifstream in("data/dataset.txt", std::ios::binary);
    if (!in) { std::cout << "[4] KLAIDA: nera data/dataset.txt (paleisk generate_dataset.py)\n"; return; }
    vector<string> lines; string line;
    while (std::getline(in, line)) lines.push_back(line + "\n");
    size_t nlines = lines.size();

    vector<size_t> sizes;
    for (size_t k = 1; k < nlines; k *= 2) sizes.push_back(k);
    sizes.push_back(nlines);

    std::ofstream csv("results/exp4_speed.csv");
    csv << "lines,bytes,v1_mean_ns,v1_min_ns,v1_max_ns,v2_mean_ns,v2_min_ns,v2_max_ns\n";
    csv << std::fixed << std::setprecision(2);

    volatile uint64_t sink = 0;
    auto measure = [&](const string& buf, int which) {
        const uint8_t* d = (const uint8_t*)buf.data(); size_t n = buf.size();
        for (int w = 0; w < 3; ++w) {
            string h = which==0?myhash::hash_v1(d,n):myhash::hash_v2(d,n);
            sink += (uint8_t)h[0];
        }
        size_t iters = 2000000 / (n + 1); if (iters < 1) iters = 1; if (iters > 200000) iters = 200000;
        double best = 1e18, worst = 0, sum = 0; int BATCH = 7;
        for (int b = 0; b < BATCH; ++b) {
            auto t0 = std::chrono::steady_clock::now();
            for (size_t it = 0; it < iters; ++it) {
                string h = which==0?myhash::hash_v1(d,n):myhash::hash_v2(d,n);
                sink += (uint8_t)h[0];
            }
            auto t1 = std::chrono::steady_clock::now();
            double per = std::chrono::duration<double, std::nano>(t1 - t0).count() / (double)iters;
            best = std::min(best, per); worst = std::max(worst, per); sum += per;
        }
        return std::array<double,3>{ sum / BATCH, best, worst };
    };

    for (size_t sz : sizes) {
        string buf;
        for (size_t i = 0; i < sz; ++i) buf += lines[i];
        auto v1 = measure(buf, 0);
        auto v2 = measure(buf, 1);
        csv << sz << "," << buf.size() << ","
            << v1[0] << "," << v1[1] << "," << v1[2] << ","
            << v2[0] << "," << v2[1] << "," << v2[2] << "\n";
    }
    std::cout << "[4] Sparta: " << sizes.size() << " dydziu -> results/exp4_speed.csv (sink=" << sink << ")\n";
}

void exp5_collisions() {
    const int PAIRS = 100000;
    vector<int> lengths = {10, 100, 500, 1000};
    std::ofstream csv("results/exp5_collisions.csv");
    csv << "version,length,pairs,pairwise_collisions,distinct_inputs,collision_groups\n";
    std::ofstream ex("results/exp5_collision_examples.txt");

    for (int L : lengths) {
        SM rng(1000 + L);
        vector<string> inputs; inputs.reserve(2*PAIRS);
        for (int p = 0; p < PAIRS; ++p) {
            string a = random_string(rng, L);
            string b = random_string(rng, L);
            if (b == a) b[0] = (b[0]==ALPHA_LO?(char)(ALPHA_LO+1):(char)ALPHA_LO);
            inputs.push_back(a); inputs.push_back(b);
        }
        for (int v = 1; v <= 2; ++v) {
            vector<string> H(inputs.size());
            for (size_t i = 0; i < inputs.size(); ++i) H[i] = HV(inputs[i], v);
            int pairwise = 0;
            for (int p = 0; p < PAIRS; ++p)
                if (H[2*p] == H[2*p+1]) { pairwise++;
                    ex << "PAIR v" << v << " L" << L << ": " << inputs[2*p] << " | " << inputs[2*p+1] << "\n"; }
            vector<int> idx(H.size()); for (size_t i=0;i<idx.size();++i) idx[i]=(int)i;
            std::sort(idx.begin(), idx.end(), [&](int a,int b){ return H[a]<H[b]; });
            int groups = 0; size_t i = 0;
            while (i < idx.size()) {
                size_t j = i; while (j < idx.size() && H[idx[j]] == H[idx[i]]) ++j;
                if (j - i > 1) {
                    bool diff = false;
                    for (size_t a=i;a<j&&!diff;++a) for (size_t b=a+1;b<j;++b)
                        if (inputs[idx[a]] != inputs[idx[b]]) { diff = true; break; }
                    if (diff) { groups++; ex << "SET v" << v << " L" << L << ": " << inputs[idx[i]] << " ...\n"; }
                }
                i = j;
            }
            vector<string> uniq(inputs); std::sort(uniq.begin(),uniq.end());
            int distinct = (int)(std::unique(uniq.begin(),uniq.end()) - uniq.begin());
            csv << "v" << v << "," << L << "," << PAIRS << "," << pairwise << ","
                << distinct << "," << groups << "\n";
        }
        std::cout << "[5] Kolizijos L=" << L << " baigta\n";
    }
    ex << "\n-- Strukturuoti bandymai --\n";
    vector<string> structured = {"abcdefghij","jihgfedcba","aaaaabbbbb","bbbbbaaaaa",
                                  "0123456789","9876543210","ababababab","bababababa"};
    for (int v=1; v<=2; ++v) {
        vector<string> H; for (auto&s:structured) H.push_back(HV(s,v));
        int coll=0;
        for (size_t a=0;a<structured.size();++a) for (size_t b=a+1;b<structured.size();++b)
            if (H[a]==H[b] && structured[a]!=structured[b]) { coll++; ex<<"STRUCT v"<<v<<": "<<structured[a]<<" = "<<structured[b]<<"\n"; }
        ex << "structured collisions v" << v << ": " << coll << "\n";
    }
    std::cout << "[5] Kolizijos -> results/exp5_collisions.csv\n";
}

void exp6_avalanche() {
    vector<int> lengths = {10, 100, 500, 1000};
    const int PER = 25000;
    std::ofstream csv("results/exp6_avalanche.csv");
    csv << std::fixed << std::setprecision(4);
    csv << "version,length,bit_min,bit_max,bit_mean,hex_min,hex_max,hex_mean\n";
    vector<long long> hist[3]; hist[1].assign(257,0); hist[2].assign(257,0);

    for (int v = 1; v <= 2; ++v) {
        int obmin=256, obmax=0; double obsum=0; int ohmin=64, ohmax=0; double ohsum=0; long long ocnt=0;
        for (int L : lengths) {
            SM rng(5000 + L + v*100000);
            int bmin=256,bmax=0; double bsum=0; int hmin=64,hmax=0; double hsum=0;
            for (int p = 0; p < PER; ++p) {
                string base = random_string(rng, L);
                string var = base;
                int pos = (int)rng.below(L);
                char nc; do { nc=(char)(ALPHA_LO+rng.below(ALPHA_N)); } while (nc==var[pos]);
                var[pos] = nc;
                string h1 = HV(base,v), h2 = HV(var,v);
                int bd = bit_diff(h1,h2), hd = hex_diff(h1,h2);
                bmin=std::min(bmin,bd); bmax=std::max(bmax,bd); bsum+=bd;
                hmin=std::min(hmin,hd); hmax=std::max(hmax,hd); hsum+=hd;
                hist[v][bd]++;
                obmin=std::min(obmin,bd); obmax=std::max(obmax,bd); obsum+=bd;
                ohmin=std::min(ohmin,hd); ohmax=std::max(ohmax,hd); ohsum+=hd; ocnt++;
            }
            double bpct=100.0/256, hpct=100.0/64;
            csv << "v"<<v<<","<<L<<","<<bmin*bpct<<","<<bmax*bpct<<","<<(bsum/PER)*bpct<<","
                << hmin*hpct<<","<<hmax*hpct<<","<<(hsum/PER)*hpct<<"\n";
        }
        double bpct=100.0/256, hpct=100.0/64;
        csv << "v"<<v<<",all,"<<obmin*bpct<<","<<obmax*bpct<<","<<(obsum/ocnt)*bpct<<","
            << ohmin*hpct<<","<<ohmax*hpct<<","<<(ohsum/ocnt)*hpct<<"\n";
        std::cout << "[6] Lavina v"<<v<<": vid. bitu skirtumas = "
                  << std::fixed << std::setprecision(2) << (obsum/ocnt)*bpct << "%\n";
    }
    std::ofstream hf("results/exp6_bithist.csv");
    hf << "bit_diff,v1_count,v2_count\n";
    for (int i = 0; i <= 256; ++i) hf << i << "," << hist[1][i] << "," << hist[2][i] << "\n";
    std::cout << "[6] Lavina -> results/exp6_avalanche.csv, results/exp6_bithist.csv\n";
}

void exp7_guessing() {
    const string target = "4242";
    std::ofstream csv("results/exp7_guessing.csv");
    csv << "version,scenario,attempts_to_first_match,total_candidates,total_time_ms,matches\n";
    csv << std::fixed << std::setprecision(3);

    string salt; { SM rng(2026); for (int i=0;i<8;++i) salt.push_back((char)rng.below(256)); }
    string secret_r; { SM rng(999); for (int i=0;i<8;++i) secret_r.push_back((char)rng.below(256)); }
    auto tohex=[](const string&b){ static const char*d="0123456789abcdef"; string o; for(unsigned char c:b){o.push_back(d[c>>4]);o.push_back(d[c&15]);} return o; };
    std::ofstream info("results/exp7_params.txt");
    info << "target=" << target << "\nsalt(hex)=" << tohex(salt) << "\nsecret_r(hex)=" << tohex(secret_r) << "\n";

    for (int v = 1; v <= 2; ++v) {
        {
            string th = HV(target, v);
            auto t0 = std::chrono::steady_clock::now();
            int matches=0, first=-1;
            for (int c = 0; c < 10000; ++c) {
                char buf[5]; snprintf(buf,5,"%04d",c);
                if (HV(string(buf), v) == th) { matches++; if(first<0) first=c+1; }
            }
            auto t1 = std::chrono::steady_clock::now();
            double ms = std::chrono::duration<double,std::milli>(t1-t0).count();
            csv << "v"<<v<<",no_salt,"<<first<<",10000,"<<ms<<","<<matches<<"\n";
        }
        {
            string th = HV(target + salt, v);
            auto t0 = std::chrono::steady_clock::now();
            int matches=0, first=-1;
            for (int c = 0; c < 10000; ++c) {
                char buf[5]; snprintf(buf,5,"%04d",c);
                if (HV(string(buf) + salt, v) == th) { matches++; if(first<0) first=c+1; }
            }
            auto t1 = std::chrono::steady_clock::now();
            double ms = std::chrono::duration<double,std::milli>(t1-t0).count();
            csv << "v"<<v<<",public_salt,"<<first<<",10000,"<<ms<<","<<matches<<"\n";
        }
        {
            string commit = HV(target + secret_r, v);
            bool verify = (HV(target + secret_r, v) == commit);
            csv << "v"<<v<<",secret_r_reveal_verify,"<<(verify?1:0)<<",NA,0,"<<(verify?1:0)<<"\n";
        }
    }
    std::cout << "[7] Spejimas -> results/exp7_guessing.csv, results/exp7_params.txt\n";
}

int main() {
    fs::create_directories("results");
    std::cout << "=== Maisos eksperimentai ===\n";
    exp1_inputs();
    exp2_format();
    exp3_determinism();
    exp4_speed();
    exp5_collisions();
    exp6_avalanche();
    exp7_guessing();
    std::cout << "=== Baigta. Rezultatai: results/ ===\n";
    return 0;
}
