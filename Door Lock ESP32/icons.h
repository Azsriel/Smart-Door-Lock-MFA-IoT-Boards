#ifndef ICONS_H
#define ICONS_H

#include <LiquidCrystal_I2C.h>

#define ICON_LOCKED_CHAR   (byte)0
#define ICON_UNLOCKED_CHAR (byte)1

#define ICON_CROSS_CHAR    (byte)88 // Capital X
#define ICON_RIGHT_ARROW   (byte)126 // https://mil.ufl.edu/4744/docs/lcdmanual/characterset.html

void init_icons(LiquidCrystal_I2C &lcd);


#endif