#ifndef COLOR_TABLE_METHODS_H
#define COLOR_TABLE_METHODS_H

#include <vision/enums/Colors.h>
/// @ingroup vision
class ColorTableMethods {
    public:
        static inline void xy2yuv(const unsigned char* image, int xIdx, int yIdx, int width, int &y, int &u, int &v) {
            int tempX = xIdx - (xIdx % 2);
            int index = ((yIdx * width) + tempX) * 2;

            u = image[index + 1];
            v = image[index + 3];
            if (xIdx % 2 == 1)
                y = image[index + 2];
            else
                y = image[index];
        }

        static inline void xy2gray(const unsigned char* image, int xIdx, int yIdx, int width, int &gray){
            int tempX = xIdx - (xIdx % 2);
            int index = ((yIdx * width) + tempX) * 2;
//            printf("Index: %d\n", index);
            if (xIdx % 2 == 1)
                gray = image[index + 2];
            else
                gray = image[index];
        }


        static inline Color xy2color(const unsigned char* image, const unsigned char* colorTable, int x, int y, int width){
            int yy,u,v;
            xy2yuv(image, x, y, width, yy, u, v);
            return yuv2color(colorTable,yy,u,v);
        }

        // does the same as xy2color above but also returns the gray value;
        static inline Color xy2color(const unsigned char* image, const unsigned char* colorTable, int x, int y, int width, int &gray){
          int u, v;
          xy2yuv(image, x, y, width, gray, u, v);
          return yuv2color(colorTable, gray, u, v);
        }

        static inline Color yuv2color(const unsigned char* colorTable, int y, int u, int v) {
            return (Color)*(colorTable + ((y >> 1 << 14) + (u >> 1 << 7) + (v >> 1)));
        }
        static inline void assignColor(unsigned char* colorTable, int y, int u, int v, Color c){
            *(colorTable + ((y >> 1 << 14) + (u >> 1 << 7) + (v >> 1))) = c;
        }
        static inline void assignColor(unsigned char* colorTable, int y, int u, int v, Color c, int yrad, int urad, int vrad, bool ignorePreviousAssignments = false){
            for(int i = y - yrad; i <= y + yrad; i++) {
                if(i < 0 || i > 255) continue;
                for(int j = u - urad; j <= u + urad; j++){
                    if(j < 0 || j > 255) continue;
                    for(int k = v - vrad; k <= v + vrad; k++) {
                        if(k < 0 || k > 255) continue;
                        if(ignorePreviousAssignments || yuv2color(colorTable,i,j,k) == c_UNDEFINED)
                            assignColor(colorTable,i,j,k,c);
                    }
                }
            }
        }
        static inline void assignColor(unsigned char* image, unsigned char* colorTable, int x, int y, int width, Color c){
            int yy,u,v;
            xy2yuv(image, x, y, width, yy, u, v);
            assignColor(colorTable,yy,u,v,c);
        }
        static inline void assignColor(unsigned char* image, unsigned char* colorTable, int x, int y, int width, Color c, int yrad, int urad, int vrad, bool ignorePreviousAssignments = false){
            int yy,u,v;
            xy2yuv(image, x, y, width, yy, u, v);
            assignColor(colorTable,yy,u,v,c,yrad,urad,vrad,ignorePreviousAssignments);
        }
};

#endif
