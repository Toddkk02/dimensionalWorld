#ifndef INVENTORY_H
#define INVENTORY_H

#include "item.h"

#define INVENTORY_SIZE 36
#define HOTBAR_SIZE 9
#define MAX_STACK 64

struct Inventory {
    Item slots[INVENTORY_SIZE];
    int selectedSlot = 0;

    // Gestione
    bool AddItem(ItemType type, int amount);
    void RemoveSelected(int amount);
    Item GetSelected();
    void SelectSlot(int index);
};

#endif
