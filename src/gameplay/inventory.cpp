#include "inventory.h"

// Aggiunge item con stacking automatico
bool Inventory::AddItem(ItemType type, int amount) {
    // Stack su slot esistenti
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (slots[i].type == type && slots[i].quantity < MAX_STACK) {
            int space = MAX_STACK - slots[i].quantity;
            int add = (amount > space) ? space : amount;
            slots[i].quantity += add;
            amount -= add;
            if (amount <= 0) return true;
        }
    }

    // Slot vuoti
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (slots[i].type == ItemType::NONE) {
            slots[i].type = type;
            slots[i].quantity = amount;
            return true;
        }
    }

    return false;
}

void Inventory::RemoveSelected(int amount) {
    Item& slot = slots[selectedSlot];
    if (slot.quantity <= 0) return;

    slot.quantity -= amount;
    if (slot.quantity <= 0) {
        slot.type = ItemType::NONE;
        slot.quantity = 0;
    }
}

Item Inventory::GetSelected() {
    return slots[selectedSlot];
}

void Inventory::SelectSlot(int index) {
    if (index >= 0 && index < HOTBAR_SIZE)
        selectedSlot = index;
}
