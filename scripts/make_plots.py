import csv, os

R = os.path.join(os.path.dirname(__file__), "..", "results")

def read_csv(name):
    with open(os.path.join(R, name), newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))

def svg_open(w, h, title):
    return [f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" '
            f'font-family="Segoe UI, Arial, sans-serif" font-size="13">',
            f'<rect width="{w}" height="{h}" fill="white"/>',
            f'<text x="{w/2}" y="24" text-anchor="middle" font-size="17" font-weight="bold">{title}</text>']

def line(x1,y1,x2,y2,col="#333",wd=1):
    return f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" stroke="{col}" stroke-width="{wd}"/>'
def text(x,y,s,anchor="start",col="#333",size=13,weight="normal"):
    return f'<text x="{x:.1f}" y="{y:.1f}" text-anchor="{anchor}" fill="{col}" font-size="{size}" font-weight="{weight}">{s}</text>'
def poly(points,col,wd=2):
    p=" ".join(f"{x:.1f},{y:.1f}" for x,y in points)
    return f'<polyline points="{p}" fill="none" stroke="{col}" stroke-width="{wd}"/>'
def rect(x,y,w,h,col):
    return f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" fill="{col}"/>'
def circle(x,y,r,col):
    return f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r}" fill="{col}"/>'

def save(parts, name):
    parts.append("</svg>")
    with open(os.path.join(R, name), "w", encoding="utf-8") as f:
        f.write("\n".join(parts))
    print("Sukurta", name)

COL = {"v1":"#c0392b", "v2":"#2471a3"}

def plot_speed():
    rows = read_csv("exp4_speed.csv")
    xs = [float(r["bytes"]) for r in rows]
    series = {"v1":[float(r["v1_mean_ns"]) for r in rows],
              "v2":[float(r["v2_mean_ns"]) for r in rows]}
    W,H=720,460; L,B,T,Rm=80,60,50,30
    pw,ph=W-L-Rm,H-B-T
    xmax=max(xs); ymax=max(max(v) for v in series.values())*1.05
    def px(x): return L + x/xmax*pw
    def py(y): return T+ph - y/ymax*ph
    s=svg_open(W,H,"Sparta: ivesties dydis vs laikas vienai maisai")
    s.append(line(L,T,L,T+ph,"#333")); s.append(line(L,T+ph,L+pw,T+ph,"#333"))
    for i in range(6):
        gx=L+pw*i/5; xv=xmax*i/5
        s.append(line(gx,T+ph,gx,T+ph+4)); s.append(text(gx,T+ph+18,f"{int(xv/1000)}k","middle",size=11))
        gy=T+ph-ph*i/5; yv=ymax*i/5
        s.append(line(L-4,gy,L,gy)); s.append(text(L-8,gy+4,f"{int(yv/1000)}","end",size=11))
    s.append(text(L+pw/2,H-16,"ivesties dydis (baitai)","middle",size=13))
    s.append(text(20,T+ph/2,"laikas / maisa (mikrosekundemis, tukst. ns)","middle",size=12))
    s.append(f'<g transform="rotate(-90 20 {T+ph/2})"></g>')
    labels={"v1":"v0.1 (mano)","v2":"v0.2 (mano)"}
    for k,ys in series.items():
        pts=[(px(x),py(y)) for x,y in zip(xs,ys)]
        s.append(poly(pts,COL[k]))
        for x,y in pts: s.append(circle(x,y,2.5,COL[k]))
    ly=T+10
    for k in ["v1","v2"]:
        s.append(rect(L+12,ly-10,14,10,COL[k])); s.append(text(L+30,ly,labels[k],size=12)); ly+=20
    save(s,"plot_speed.svg")

def plot_hist():
    rows=read_csv("exp6_bithist.csv")
    bins=[int(r["bit_diff"]) for r in rows]
    v1=[int(r["v1_count"]) for r in rows]
    v2=[int(r["v2_count"]) for r in rows]
    W,H=720,460; L,B,T,Rm=70,60,50,30
    pw,ph=W-L-Rm,H-B-T
    ymax=max(max(v1),max(v2))*1.05
    def px(b): return L + b/256*pw
    def py(c): return T+ph - c/ymax*ph
    bw=pw/256
    s=svg_open(W,H,"Lavinos efektas: skirtingu bitu pasiskirstymas (100 000 poru)")
    s.append(line(L,T,L,T+ph,"#333")); s.append(line(L,T+ph,L+pw,T+ph,"#333"))
    for i in range(9):
        b=32*i; gx=px(b)
        s.append(line(gx,T+ph,gx,T+ph+4)); s.append(text(gx,T+ph+18,str(b),"middle",size=11))
    s.append(text(L+pw/2,H-16,"skirtingu bitu skaicius (is 256)","middle"))
    s.append(line(px(128),T,px(128),T+ph,"#999",1))
    s.append(text(px(128)+4,T+14,"128 = 50%","start",col="#666",size=11))
    for c,col in [(v1,COL["v1"]),(v2,COL["v2"])]:
        for b,val in zip(bins,c):
            if val<=0: continue
            h=(T+ph)-py(val)
            s.append(f'<rect x="{px(b):.1f}" y="{py(val):.1f}" width="{max(bw,1):.1f}" height="{h:.1f}" fill="{col}" fill-opacity="0.6"/>')
    ly=T+10
    for k,lab in [("v1","v0.1 (mano) ~11%"),("v2","v0.2 (mano) ~50%")]:
        s.append(rect(L+12,ly-10,14,10,COL[k])); s.append(text(L+30,ly,lab,size=12)); ly+=20
    save(s,"plot_avalanche_hist.svg")

def plot_bylen():
    rows=[r for r in read_csv("exp6_avalanche.csv") if r["length"]!="all"]
    lens=["10","100","500","1000"]
    v1=[float(next(r for r in rows if r["version"]=="v1" and r["length"]==L)["bit_mean"]) for L in lens]
    v2=[float(next(r for r in rows if r["version"]=="v2" and r["length"]==L)["bit_mean"]) for L in lens]
    W,H=620,440; L0,B,T,Rm=70,60,50,30
    pw,ph=W-L0-Rm,H-B-T
    ymax=60
    def py(v): return T+ph - v/ymax*ph
    s=svg_open(W,H,"Vidutinis bitu skirtumas pagal ivesties ilgi")
    s.append(line(L0,T,L0,T+ph,"#333")); s.append(line(L0,T+ph,L0+pw,T+ph,"#333"))
    for i in range(7):
        yv=ymax*i/6; gy=py(yv)
        s.append(line(L0-4,gy,L0,gy)); s.append(text(L0-8,gy+4,f"{int(yv)}%","end",size=11))
    s.append(line(L0,py(50),L0+pw,py(50),"#999",1)); s.append(text(L0+pw,py(50)-4,"50% (ideal)","end",col="#666",size=11))
    gw=pw/len(lens); bw=gw*0.32
    for i,L in enumerate(lens):
        cx=L0+gw*(i+0.5)
        s.append(rect(cx-bw-3,py(v1[i]),bw,(T+ph)-py(v1[i]),COL["v1"]))
        s.append(rect(cx+3,py(v2[i]),bw,(T+ph)-py(v2[i]),COL["v2"]))
        s.append(text(cx,T+ph+18,f"L={L}","middle",size=12))
        s.append(text(cx-bw/2-3,py(v1[i])-4,f"{v1[i]:.0f}","middle",col=COL['v1'],size=10))
        s.append(text(cx+bw/2+3,py(v2[i])-4,f"{v2[i]:.0f}","middle",col=COL['v2'],size=10))
    ly=T+10
    for k,lab in [("v1","v0.1"),("v2","v0.2")]:
        s.append(rect(L0+12,ly-10,14,10,COL[k])); s.append(text(L0+30,ly,lab,size=12)); ly+=20
    save(s,"plot_avalanche_bylen.svg")

if __name__=="__main__":
    plot_speed(); plot_hist(); plot_bylen()
    print("Visi grafikai sukurti results/ kataloge.")
