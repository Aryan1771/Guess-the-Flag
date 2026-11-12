#define UNICODE
#define _UNICODE
#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <random>
#include <ctime>
#include <sstream>

using namespace Gdiplus;
using std::wstring;

namespace {
constexpr int W = 1120, H = 760;
const Color BG(255, 10, 14, 24), PANEL(255, 19, 25, 38), PANEL2(255, 25, 33, 48);
const Color TEXT(255, 238, 242, 250), MUTED(255, 139, 151, 173), ACCENT(255, 115, 224, 190);
const Color RED(255, 255, 111, 128), GREEN(255, 102, 222, 170), LINE(255, 43, 53, 71);
enum class Screen { Home, Playing, Result };
struct Country { const wchar_t* name; int style; std::array<Color,4> c; };
const std::vector<Country> countries = {
 {L"Japan",0,{Color(255,255,255,255),Color(255,188,0,45),Color(255,255,255,255),Color(255,255,255,255)}},
 {L"France",1,{Color(255,0,85,164),Color(255,255,255,255),Color(255,239,65,53),Color(255,0,85,164)}},
 {L"Germany",2,{Color(255,0,0,0),Color(255,221,0,0),Color(255,255,206,0),Color(255,0,0,0)}},
 {L"Italy",1,{Color(255,0,146,70),Color(255,255,255,255),Color(255,206,43,55),Color(255,0,146,70)}},
 {L"Netherlands",2,{Color(255,174,28,40),Color(255,255,255,255),Color(255,33,70,139),Color(255,174,28,40)}},
 {L"Ukraine",2,{Color(255,0,87,183),Color(255,255,213,0),Color(255,0,87,183),Color(255,255,213,0)}},
 {L"Poland",2,{Color(255,255,255,255),Color(255,220,20,60),Color(255,255,255,255),Color(255,220,20,60)}},
 {L"Ireland",1,{Color(255,22,155,98),Color(255,255,255,255),Color(255,255,136,62),Color(255,22,155,98)}},
 {L"Sweden",3,{Color(255,0,106,167),Color(255,254,204,0),Color(255,0,106,167),Color(255,254,204,0)}},
 {L"Denmark",4,{Color(255,198,12,48),Color(255,255,255,255),Color(255,198,12,48),Color(255,255,255,255)}},
 {L"Czechia",5,{Color(255,255,255,255),Color(255,215,20,26),Color(255,17,69,126),Color(255,255,255,255)}},
 {L"Greece",6,{Color(255,13,94,175),Color(255,255,255,255),Color(255,13,94,175),Color(255,255,255,255)}},
 {L"Switzerland",7,{Color(255,218,37,29),Color(255,255,255,255),Color(255,218,37,29),Color(255,255,255,255)}},
 {L"Turkey",8,{Color(255,227,10,23),Color(255,255,255,255),Color(255,227,10,23),Color(255,255,255,255)}},
 {L"Brazil",9,{Color(255,0,156,59),Color(255,255,223,0),Color(255,0,39,118),Color(255,255,255,255)}},
 {L"Canada",10,{Color(255,255,255,255),Color(255,213,0,50),Color(255,255,255,255),Color(255,213,0,50)}},
 {L"United States",11,{Color(255,179,25,66),Color(255,255,255,255),Color(255,60,59,110),Color(255,179,25,66)}},
 {L"South Korea",12,{Color(255,255,255,255),Color(255,205,46,58),Color(255,0,71,160),Color(255,255,255,255)}},
 {L"India",13,{Color(255,255,153,51),Color(255,255,255,255),Color(255,19,136,8),Color(255,0,0,128)}},
 {L"Mexico",1,{Color(255,0,104,71),Color(255,255,255,255),Color(255,206,17,38),Color(255,0,104,71)}}
};
struct Box { int x,y,w,h; bool has(int a,int b) const { return a>=x&&a<x+w&&b>=y&&b<y+h; } };
HWND hwnd; ULONG_PTR gdip; Screen screen=Screen::Home; std::mt19937 rng((unsigned)std::time(nullptr));
std::vector<int> rounds, choices; int roundNo=0, score=0, selected=-1, hover=-1; bool locked=false; DWORD answerAt=0;
Box startBtn{390,574,340,62}, nextBtn{746,615,250,56}, homeBtn{390,574,340,62};
std::array<Box,4> optionBoxes{{{383,480,350,62},{747,480,350,62},{383,554,350,62},{747,554,350,62}}};

void brush(Graphics& g, Color c, int x,int y,int w,int h){ SolidBrush b(c); g.FillRectangle(&b,x,y,w,h); }
void roundPath(GraphicsPath& p,int x,int y,int w,int h,int r){p.Reset();p.AddArc(x,y,2*r,2*r,180,90);p.AddArc(x+w-2*r,y,2*r,2*r,270,90);p.AddArc(x+w-2*r,y+h-2*r,2*r,2*r,0,90);p.AddArc(x,y+h-2*r,2*r,2*r,90,90);p.CloseFigure();}
void rounded(Graphics& g, Color c, int x,int y,int w,int h,int r=16){ GraphicsPath p;roundPath(p,x,y,w,h,r);SolidBrush b(c);g.FillPath(&b,&p); }
void outline(Graphics& g, Color c, int x,int y,int w,int h,int r=16,float sw=1){ GraphicsPath p;roundPath(p,x,y,w,h,r);Pen pen(c,sw);g.DrawPath(&pen,&p); }
void txt(Graphics& g, const wchar_t* s, int x,int y,int w,int h,float size,Color c,bool bold=false, int align=0){
 FontFamily fam(L"Segoe UI"); Font f(&fam,size,bold?FontStyleBold:FontStyleRegular,UnitPixel); SolidBrush b(c);
 StringFormat sf; sf.SetLineAlignment(StringAlignmentCenter); sf.SetAlignment(align==1?StringAlignmentCenter:align==2?StringAlignmentFar:StringAlignmentNear);
 RectF r((REAL)x,(REAL)y,(REAL)w,(REAL)h); g.DrawString(s,-1,&f,r,&sf,&b);
}
void txt(Graphics& g, const wstring& s, int x,int y,int w,int h,float size,Color c,bool bold=false,int align=0){txt(g,s.c_str(),x,y,w,h,size,c,bold,align);}
void flag(Graphics& g,int idx,int x,int y,int w,int h){
 const Country& co=countries[idx]; GraphicsState st=g.Save(); g.SetSmoothingMode(SmoothingModeAntiAlias);
 GraphicsPath path; roundPath(path,x,y,w,h,10); g.SetClip(&path);
 brush(g,co.c[0],x,y,w,h);
 auto stripe=[&](int n, bool horizontal){ for(int i=0;i<n;i++) brush(g,co.c[(i+1)%4], horizontal?x+(w*i)/n:x, horizontal?y:y+(h*i)/n, horizontal?w/n+1:w, horizontal?h:h/n+1); };
 switch(co.style){
 case 0:{ SolidBrush b(co.c[1]); g.FillEllipse(&b,x+w*.37f,y+h*.19f,w*.26f,h*.62f); break; }
 case 1: stripe(3,false); break;
 case 2: stripe(3,true); break;
 case 3: stripe(1,false); brush(g,co.c[1],x+w*.30f,y,w*.17f,h); break;
 case 4: stripe(1,false); brush(g,co.c[1],x+w*.32f,y,w*.15f,h); break;
 case 5: { brush(g,co.c[1],x,y+h*.5f,w,h*.5f); PointF pts[3]={{(REAL)x,(REAL)y},{x+w*.46f,y+h*.5f},{(REAL)x,(REAL)(y+h)}}; SolidBrush b(co.c[2]); g.FillPolygon(&b,pts,3); break; }
 case 6: stripe(5,true); brush(g,co.c[1],x,y+h*.38f,w,h*.15f); brush(g,co.c[1],x+w*.23f,y,w*.12f,h*.63f); brush(g,co.c[0],x,y,w*.39f,h*.44f); break;
 case 7: brush(g,co.c[1],x+w*.38f,y+h*.24f,w*.24f,h*.52f); brush(g,co.c[1],x+w*.24f,y+h*.42f,w*.52f,h*.16f); break;
 case 8:{ SolidBrush b(co.c[1]);g.FillEllipse(&b,x+w*.27f,y+h*.20f,w*.46f,h*.60f);SolidBrush r(co.c[0]);g.FillEllipse(&r,x+w*.33f,y+h*.20f,w*.4f,h*.60f);break;}
 case 9:{PointF p[4]={{x+w*.5f,y+h*.12f},{x+w*.9f,y+h*.5f},{x+w*.5f,y+h*.88f},{x+w*.1f,y+h*.5f}};SolidBrush b(co.c[1]);g.FillPolygon(&b,p,4);SolidBrush c(co.c[2]);g.FillEllipse(&c,x+w*.36f,y+h*.25f,w*.28f,h*.5f);break;}
 case 10: brush(g,co.c[1],x,y,w*.25f,h);brush(g,co.c[1],x+w*.75f,y,w*.25f,h);txt(g,L"✦",x+w*.35f,y+h*.15f,w*.3f,h*.7f,29,co.c[1],true,1);break;
 case 11: stripe(7,true);brush(g,co.c[2],x,y,w*.43f,h*.54f);txt(g,L"✦  ✦\n✦  ✦",x+3,y+1,w*.4f,h*.48f,10,Color(255,255,255,255),true,1);break;
 case 12:{SolidBrush r(co.c[1]),b(co.c[2]);g.FillEllipse(&r,x+w*.38f,y+h*.27f,w*.24f,h*.24f);g.FillEllipse(&b,x+w*.38f,y+h*.49f,w*.24f,h*.24f);break;}
 case 13: stripe(3,true);{Pen p(Color(255,0,0,128),2);g.DrawEllipse(&p,x+w*.46f,y+h*.40f,w*.08f,h*.20f);}break;
 }
 g.Restore(st); g.SetSmoothingMode(SmoothingModeAntiAlias); outline(g,Color(60,255,255,255),x,y,w,h,10,1);
}
void newGame(){ std::vector<int> pool(countries.size()); for(int i=0;i<(int)pool.size();i++)pool[i]=i;std::shuffle(pool.begin(),pool.end(),rng);rounds.assign(pool.begin(),pool.begin()+7);roundNo=score=0;selected=-1;locked=false;screen=Screen::Playing; }
void makeChoices(){ choices={rounds[roundNo]};std::vector<int> pool;for(int i=0;i<(int)countries.size();i++)if(i!=rounds[roundNo])pool.push_back(i);std::shuffle(pool.begin(),pool.end(),rng);choices.insert(choices.end(),pool.begin(),pool.begin()+3);std::shuffle(choices.begin(),choices.end(),rng);selected=-1;locked=false; }
void roundStart(){makeChoices();InvalidateRect(hwnd,nullptr,FALSE);}
void drawBg(Graphics& g){ brush(g,BG,0,0,W,H); for(int i=0;i<8;i++){SolidBrush b(Color(7,105,210,190));g.FillEllipse(&b,870+i*25,80+i*17,300,300);} }
void drawHome(Graphics& g){
 txt(g,L"FIELD NOTES     /     GEOGRAPHY DESK",70,40,470,25,12,ACCENT,true);
 txt(g,L"Find the flag.",70,105,660,66,48,TEXT,true); txt(g,L"A little geography. A quiet room. Seven chances to get it right.",73,178,820,32,17,MUTED);
 rounded(g,PANEL,70,262,980,260,22);txt(g,L"THE NIGHT SHIFT",103,292,300,24,12,ACCENT,true);
 txt(g,L"The desk lamp hums. A globe turns slowly on the monitor.\nSomewhere in its atlas, seven flags are waiting to be found.",103,334,580,76,22,TEXT,true);
 txt(g,L"Choose a country for each flag. Take your time; the score is yours to earn.",103,431,650,30,15,MUTED);
 rounded(g,Color(255,30,43,59),800,300,190,170,18);txt(g,L"07",800,317,190,76,56,ACCENT,true,1);txt(g,L"FLAGS  /  ONE RUN",800,395,190,25,12,MUTED,true,1);
 rounded(g,Color(255,28,48,62),70,536,980,2,1);txt(g,L"7 FLAGS",75,550,170,22,12,MUTED,true);txt(g,L"4 COUNTRY OPTIONS",275,550,230,22,12,MUTED,true);txt(g,L"NO TIMER",535,550,180,22,12,MUTED,true);
 rounded(g,ACCENT,startBtn.x,startBtn.y,startBtn.w,startBtn.h,14);txt(g,L"START A ROUND     →",startBtn.x,startBtn.y,startBtn.w,startBtn.h,16,Color(255,9,24,29),true,1);
 txt(g,L"A SMALL FLAG QUIZ  ·  ORIGINAL FAN-MADE GAME",0,700,W,22,10,Color(255,79,92,111),true,1);
}
void drawGame(Graphics& g){
 txt(g,L"FIELD NOTES     /     GEOGRAPHY DESK",55,25,440,22,11,ACCENT,true);txt(g,L"FIND THE FLAG",55,57,300,26,19,TEXT,true);
 wstring prog=L"ROUND  "+(roundNo+1<10?L"0":L"")+std::to_wstring(roundNo+1)+L"  /  07";txt(g,prog,790,53,275,24,13,MUTED,true,2);
 rounded(g,LINE,55,94,1010,5,3);rounded(g,ACCENT,55,94,1010*(roundNo+1)/7,5,3);
 rounded(g,PANEL,55,122,1010,324,20); txt(g,L"WHICH COUNTRY DOES THIS FLAG BELONG TO?",88,146,720,22,12,MUTED,true);
 flag(g,rounds[roundNo],370,188,380,218);
 txt(g,L"SELECT ONE ANSWER",55,462,400,22,12,MUTED,true);
 for(int i=0;i<4;i++){Box b=optionBoxes[i]; Color fill=PANEL;Color edge=LINE; if(locked&&choices[i]==rounds[roundNo]){fill=Color(255,22,57,48);edge=GREEN;}else if(locked&&selected==i){fill=Color(255,60,32,43);edge=RED;}else if(hover==i){fill=PANEL2;edge=ACCENT;}rounded(g,fill,b.x,b.y,b.w,b.h,13);outline(g,edge,b.x,b.y,b.w,b.h,13,1.3f);wstring label=std::to_wstring(i+1)+L"     "+countries[choices[i]].name;txt(g,label,b.x+18,b.y,b.w-36,b.h,17,TEXT,true);}
 if(locked){const bool good=choices[selected]==rounds[roundNo]; txt(g,good?L"THAT'S IT. NICE EYE.":(wstring(L"THE FLAG WAS ")+countries[rounds[roundNo]].name),55,632,600,28,14,good?GREEN:RED,true);rounded(g,ACCENT,nextBtn.x,nextBtn.y,nextBtn.w,nextBtn.h,13);txt(g,roundNo==6?L"SEE YOUR RESULTS   →":L"NEXT FLAG   →",nextBtn.x,nextBtn.y,nextBtn.w,nextBtn.h,14,Color(255,9,24,29),true,1);}
 else txt(g,L"TIP:  PRESS 1, 2, 3 OR 4 TO ANSWER",55,632,500,25,12,MUTED,true);
 txt(g,L"SCORE  "+std::to_wstring(score),870,630,190,28,13,ACCENT,true,2);
}
void drawResult(Graphics& g){
 txt(g,L"FIELD NOTES     /     GEOGRAPHY DESK",70,40,470,25,12,ACCENT,true);txt(g,L"Your atlas is getting fuller.",70,109,850,65,40,TEXT,true);
 txt(g,L"Seven flags later, here's how the journey went.",73,185,750,30,17,MUTED);
 rounded(g,PANEL,70,256,980,248,22);txt(g,std::to_wstring(score),105,282,250,125,82,ACCENT,true);txt(g,L"OUT OF 7",111,408,240,28,14,MUTED,true);
 const wchar_t* msg=score==7?L"PERFECT RUN. THE ATLAS IS YOURS.":score>=5?L"A SHARP EYE. KEEP EXPLORING.":score>=3?L"A GOOD START. THERE'S MORE TO DISCOVER.":L"THE WORLD IS BIG. GIVE IT ANOTHER GO.";
 txt(g,msg,390,322,580,42,19,TEXT,true);
 txt(g,L"RUN SUMMARY",70,535,240,24,12,ACCENT,true);
 for(int i=0;i<7;i++){int x=70+(i%4)*248,y=573+(i/4)*53;rounded(g,PANEL2,x,y,228,42,10);txt(g,L"0"+std::to_wstring(i+1)+L"   "+countries[rounds[i]].name,x+12,y,206,42,13,TEXT,true);}
 rounded(g,ACCENT,720,650,330,56,13);txt(g,L"PLAY AGAIN     →",720,650,330,56,15,Color(255,9,24,29),true,1);
}
void paint(){ PAINTSTRUCT ps;HDC h=BeginPaint(hwnd,&ps);Graphics g(h);g.SetSmoothingMode(SmoothingModeAntiAlias);g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);drawBg(g);if(screen==Screen::Home)drawHome(g);else if(screen==Screen::Playing)drawGame(g);else drawResult(g);EndPaint(hwnd,&ps); }
void choose(int i){if(screen!=Screen::Playing||locked||i<0||i>3)return;selected=i;locked=true;answerAt=GetTickCount();if(choices[i]==rounds[roundNo])score++;InvalidateRect(hwnd,nullptr,FALSE);SetTimer(hwnd,1,450,FALSE);}
LRESULT CALLBACK proc(HWND h,UINT m,WPARAM wp,LPARAM lp){switch(m){case WM_PAINT:paint();return 0;case WM_ERASEBKGND:return 1;case WM_MOUSEMOVE:{int x=GET_X_LPARAM(lp),y=GET_Y_LPARAM(lp),old=hover;hover=-1;if(screen==Screen::Playing&&!locked)for(int i=0;i<4;i++)if(optionBoxes[i].has(x,y))hover=i;if(old!=hover)InvalidateRect(h,nullptr,FALSE);return 0;}case WM_LBUTTONUP:{int x=GET_X_LPARAM(lp),y=GET_Y_LPARAM(lp);if(screen==Screen::Home&&startBtn.has(x,y)){newGame();roundStart();}else if(screen==Screen::Playing){for(int i=0;i<4;i++)if(optionBoxes[i].has(x,y))choose(i);if(locked&&nextBtn.has(x,y)){if(roundNo==6)screen=Screen::Result;else{roundNo++;roundStart();}}}else if(screen==Screen::Result&&Box{720,650,330,56}.has(x,y)){screen=Screen::Home;}InvalidateRect(h,nullptr,FALSE);return 0;}case WM_KEYDOWN:if(screen==Screen::Home&&(wp==VK_RETURN||wp==VK_SPACE)){newGame();roundStart();}else if(screen==Screen::Playing){if(wp>=L'1'&&wp<=L'4')choose((int)wp-(int)L'1');if(locked&&(wp==VK_RETURN||wp==VK_SPACE)){if(roundNo==6)screen=Screen::Result;else{roundNo++;roundStart();}}}else if(screen==Screen::Result&&(wp==VK_RETURN||wp==VK_SPACE))screen=Screen::Home;InvalidateRect(h,nullptr,FALSE);return 0;case WM_TIMER:KillTimer(h,1);InvalidateRect(h,nullptr,FALSE);return 0;case WM_DESTROY:GdiplusShutdown(gdip);PostQuitMessage(0);return 0;}return DefWindowProcW(h,m,wp,lp);}
}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,LPWSTR,int show){GdiplusStartupInput si;GdiplusStartup(&gdip,&si,nullptr);WNDCLASSW wc{};wc.lpfnWndProc=proc;wc.hInstance=inst;wc.lpszClassName=L"FindTheFlagWindow";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=nullptr;RegisterClassW(&wc);RECT r{0,0,W,H};AdjustWindowRect(&r,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,FALSE);hwnd=CreateWindowExW(0,wc.lpszClassName,L"Find the Flag — Field Notes",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,nullptr,nullptr,inst,nullptr);ShowWindow(hwnd,show);UpdateWindow(hwnd);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return (int)msg.wParam;}
