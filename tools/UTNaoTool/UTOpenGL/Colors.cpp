#include "Colors.h"

using namespace std;

RGB Colors::White = TORGB(255,255,255);
RGB Colors::Gray = TORGB(125,125,125);
RGB Colors::Black = TORGB(0,0,0);

RGB Colors::Red = TORGB(255,0,0);
RGB Colors::Orange = TORGB(255,125,0);
RGB Colors::Yellow = TORGB(255,255,0);
RGB Colors::Green = TORGB(0,255,0);
RGB Colors::Blue = TORGB(0,0,255);
RGB Colors::Indigo = TORGB(0,0,153);
RGB Colors::Violet = TORGB(160,32,240);
RGB Colors::Brown = TORGB(205,92,92);
RGB Colors::Cyan = TORGB(0,255,255);
RGB Colors::Magenta = TORGB(255,0,255);
RGB Colors::Pink = Colors::Magenta;

RGB Colors::LightRed = TORGB(255,150,150);
RGB Colors::LightOrange = TORGB(255,178,150);
RGB Colors::LightYellow = TORGB(255,255,150);
RGB Colors::LightGreen = TORGB(102,255,150);
RGB Colors::LightBlue = TORGB(51,153,255);
RGB Colors::LightIndigo = TORGB(150,150,255);
RGB Colors::LightViolet = TORGB(153,51,255);
RGB Colors::LightBrown = TORGB(210,180,140);
RGB Colors::LightCyan = TORGB(150,255,255);
RGB Colors::LightMagenta = TORGB(255,150,255);

vector<RGB> Colors::StandardColors =  { Blue, Red, Yellow, Green, Indigo, Brown, Cyan, Magenta, Orange, Violet };
vector<RGB> Colors::LightColors = { LightBlue, LightRed, LightYellow, LightGreen, LightIndigo, LightBrown, LightCyan, LightMagenta, LightOrange, LightViolet };
