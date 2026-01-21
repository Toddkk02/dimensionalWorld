#include "inventory.h"
#include "raylib.h"
#include <cstdio>

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

// ========== UI RENDERING ==========

void Inventory::DrawHotbar() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    
    int slotSize = 50;
    int spacing = 5;
    int totalWidth = HOTBAR_SIZE * (slotSize + spacing) - spacing;
    int startX = (screenW - totalWidth) / 2;
    int startY = screenH - slotSize - 20;
    
    for (int i = 0; i < HOTBAR_SIZE; i++) {
        int x = startX + i * (slotSize + spacing);
        int y = startY;
        
        // Sfondo slot
        Color bgColor = (i == selectedSlot) ? DARKGRAY : GRAY;
        DrawRectangle(x, y, slotSize, slotSize, bgColor);
        DrawRectangleLines(x, y, slotSize, slotSize, BLACK);
        
        // Item
        if (slots[i].type != ItemType::NONE) {
            Color itemColor = GetItemColor(slots[i].type);
            DrawRectangle(x + 5, y + 5, slotSize - 10, slotSize - 10, itemColor);
            
            // Quantità
            char buf[8];
            snprintf(buf, sizeof(buf), "%d", slots[i].quantity);
            DrawText(buf, x + slotSize - 15, y + slotSize - 15, 10, WHITE);
        }
        
        // Numero slot
        char numBuf[4];
        snprintf(numBuf, sizeof(numBuf), "%d", i + 1);
        DrawText(numBuf, x + 2, y + 2, 10, LIGHTGRAY);
    }
}

void Inventory::DrawFullInventory() {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    
    int slotSize = 50;
    int spacing = 5;
    int cols = 9;
    int rows = 4;
    
    int totalWidth = cols * (slotSize + spacing) - spacing;
    int totalHeight = rows * (slotSize + spacing) - spacing;
    int startX = (screenW - totalWidth) / 2;
    int startY = (screenH - totalHeight) / 2;
    
    // Sfondo
    DrawRectangle(startX - 20, startY - 40, totalWidth + 40, totalHeight + 80, Fade(BLACK, 0.8f));
    DrawText("INVENTORY", startX, startY - 30, 20, WHITE);
    
    // Disegna tutti gli slot
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int index = row * cols + col;
            if (index >= INVENTORY_SIZE) break;
            
            int x = startX + col * (slotSize + spacing);
            int y = startY + row * (slotSize + spacing);
            
            // Sfondo slot
            Color bgColor = (index == selectedSlot) ? DARKGRAY : GRAY;
            DrawRectangle(x, y, slotSize, slotSize, bgColor);
            DrawRectangleLines(x, y, slotSize, slotSize, BLACK);
            
            // Item
            if (slots[index].type != ItemType::NONE) {
                Color itemColor = GetItemColor(slots[index].type);
                DrawRectangle(x + 5, y + 5, slotSize - 10, slotSize - 10, itemColor);
                
                // Quantità
                char buf[8];
                snprintf(buf, sizeof(buf), "%d", slots[index].quantity);
                DrawText(buf, x + slotSize - 15, y + slotSize - 15, 10, WHITE);
            }
        }
    }
    
    DrawText("Press E to close", startX, startY + totalHeight + 10, 20, LIGHTGRAY);
}