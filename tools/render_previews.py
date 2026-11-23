"""Render static design previews; the device UI itself is native LVGL."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

S = 2
W, H = 800, 480
OUT = Path(__file__).resolve().parents[1] / "docs"
OUT.mkdir(exist_ok=True)
BG = "#061015"; PANEL = "#0D1B22"; PANEL2 = "#11262D"; BORDER = "#23434B"
TEXT = "#F1F8F5"; MUTED = "#8CA7AD"; GREEN = "#75F04B"; CYAN = "#32DDE1"; YELLOW = "#FFD84A"

def font(size, bold=False):
    name = "segoeuib.ttf" if bold else "segoeui.ttf"
    return ImageFont.truetype(str(Path("C:/Windows/Fonts") / name), size * S)

def box(draw, xy, fill=PANEL, outline=BORDER, radius=12, width=1):
    draw.rounded_rectangle(tuple(v*S for v in xy), radius*S, fill, outline, width*S)

def txt(draw, xy, text, size=14, fill=TEXT, bold=False, anchor=None):
    draw.text((xy[0]*S, xy[1]*S), text, font=font(size, bold), fill=fill, anchor=anchor)

def base(title="ELECTRIC GOLF CART"):
    im = Image.new("RGB", (W*S, H*S), BG); d = ImageDraw.Draw(im)
    box(d, (12, 12, 46, 44), "#123526", GREEN, 8)
    txt(d, (29, 28), "EV", 16, GREEN, True, "mm")
    txt(d, (58, 5), title, 20, TEXT, True)
    txt(d, (59, 32), "VEHICLE MONITOR", 12, MUTED)
    box(d, (570, 11, 674, 43), "#17362B", "#2E6745", 16)
    txt(d, (622, 27), "DEMO MODE", 12, GREEN, False, "mm")
    txt(d, (689, 15), "10:24 AM", 16)
    d.line((12*S, 56*S, 788*S, 56*S), fill=BORDER, width=S)
    return im, d

def nav(d, active):
    box(d, (0,424,800,480), "#09171C", BORDER, 0)
    names = ["OVERVIEW", "SYSTEMS", "V2V LAB"]
    widths = [248,248,264]; x = 8
    for i, (name,w) in enumerate(zip(names,widths)):
        color = GREEN if i == active else MUTED
        box(d, (x,430,x+w,472), "#174534" if i == active else BG,
            GREEN if i == active else BORDER, 10)
        txt(d, (x+w/2,451), name, 16, color, i == active, "mm")
        x += w+7

def overview():
    im,d = base()
    cards = [(16,"SPEED","24.5","km/h",CYAN),(126,"EST. RANGE","42","km",GREEN),(236,"BATTERY","2.4","kW",YELLOW)]
    for y,title,value,unit,accent in cards:
        box(d,(12,58+y,158,58+y+102)); box(d,(12,58+y,16,58+y+102),accent,accent,2)
        txt(d,(28,58+y+12),title,14,MUTED); txt(d,(27,58+y+36),value,32,TEXT,True); txt(d,(114,58+y+53),unit,12,MUTED)
    box(d,(168,74,788,404)); txt(d,(186,86),"LIVE POWER FLOW",14,MUTED)
    box(d,(641,82,770,108),"#163B2C","#275E3E",13); txt(d,(705,95),"ALL NORMAL",12,GREEN,False,"mm")
    txt(d,(186,128),"BATTERY (SOC)",14,MUTED); txt(d,(185,149),"78%",48,GREEN,True)
    txt(d,(187,211),"7.8 / 10.0 kWh",16); box(d,(187,243,357,259),"#1D3538","#1D3538",8)
    box(d,(187,243,320,259),GREEN,GREEN,8); txt(d,(187,274),"51.8 V  |  32.6 C",14,MUTED)
    txt(d,(187,307),"DISCHARGING",14,GREEN); txt(d,(187,335),"10.0 kWh traction pack",12,MUTED); txt(d,(187,356),"SOH 96%",12,MUTED)
    d.line((375*S,122*S,375*S,378*S),fill=BORDER,width=S)
    d.line((509*S,205*S,509*S,265*S),fill=GREEN,width=4*S)
    d.line((524*S,305*S,617*S,305*S),fill=GREEN,width=4*S)
    d.line((617*S,348*S,524*S,348*S),fill=CYAN,width=2*S)
    for cx,cy in [(509,230),(583,305),(560,348)]: d.ellipse(((cx-4)*S,(cy-4)*S,(cx+4)*S,(cy+4)*S),fill=GREEN)
    for xy,name,value,accent in [((454,122,564,206),"SOLAR","1.2 kW",YELLOW),((399,264,525,350),"PACK","2.4 kW",GREEN),((617,264,763,350),"MOTOR","2.8 kW",CYAN)]:
        box(d,xy,PANEL2,accent,10); cx=(xy[0]+xy[2])/2
        txt(d,(cx,xy[1]+14),name,12,accent,False,"mm"); txt(d,(cx,xy[1]+53),value,24,TEXT,True,"mm")
    txt(d,(557,363),"REGEN 0.4 kW",12,CYAN); nav(d,0)
    im.save(OUT/"overview-preview.png")

def v2v():
    im,d = base("V2V CHARGING LAB")
    box(d,(12,74,788,129),"#382B14","#8A6928",10)
    txt(d,(30,82),"WORK IN PROGRESS - V2V CHARGING",18,YELLOW,True)
    txt(d,(30,108),"Concept screen only - no charging outputs or hardware are controlled.",12)
    for x,name,soc,accent in [(24,"CART A","78%",GREEN),(562,"CART B","34%",CYAN)]:
        box(d,(x,151,x+214,353),PANEL2,accent,14); txt(d,(x+18,166),name,20,TEXT,True)
        txt(d,(x+18,197),"TRACTION PACK",12,MUTED); txt(d,(x+18,220),soc,40,accent,True)
        box(d,(x+18,278,x+196,294),"#1D3538","#1D3538",8)
        box(d,(x+18,278,x+18+int(178*int(soc[:-1])/100),294),accent,accent,8)
        txt(d,(x+18,314),"SIMULATED BMS",12,MUTED)
    txt(d,(265,152),"RECOMMENDED ENERGY PATH",12,MUTED); txt(d,(272,180),"CART A  >  CART B",20,GREEN,True)
    d.line((251*S,237*S,548*S,237*S),fill=GREEN,width=5*S)
    for x in [290,355,420,485]: d.ellipse(((x-5)*S,232*S,(x+5)*S,242*S),fill=GREEN)
    txt(d,(290,262),"Higher SOC donates to lower SOC",14,MUTED); txt(d,(298,293),"0.0 kW - NOT CONNECTED",14,"#FFB44A")
    box(d,(310,325,490,365),"#203A3D",CYAN,10); txt(d,(400,345),"SWAP DEMO SOC",14,TEXT,False,"mm")
    txt(d,(85,381),"Isolated DC/DC + compatible BMS + precharge + interlocks required.",12,MUTED)
    nav(d,2); im.save(OUT/"v2v-preview.png")

if __name__ == "__main__":
    overview(); v2v(); print(f"Previews written to {OUT}")
