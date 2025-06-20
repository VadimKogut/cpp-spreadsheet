#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    auto [it, inserted] = cells_.try_emplace(pos, nullptr);
    if (inserted) {
        it->second = std::make_unique<Cell>(*this);
    }

    it->second->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetCellPtr(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetCellPtr(pos);
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position");
    }

    auto it = cells_.find(pos);
    if (it != cells_.end()) {
        it->second->Clear();
        if (!it->second->IsReferenced() && it->second->GetText().empty()) {
            cells_.erase(it);
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size result{0, 0};

    for (const auto& [pos, cell] : cells_) {
        if (cell && !cell->GetText().empty()) {
            result.rows = std::max(result.rows, pos.row + 1);
            result.cols = std::max(result.cols, pos.col + 1);
        }
    }

    return result;
}

void Sheet::Print(std::ostream& output, const std::function<void(const Cell&, std::ostream&)>& printer) const {
    const auto size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << "\t";
            }

            Position pos{row, col};
            if (auto it = cells_.find(pos); it != cells_.end() && it->second && !it->second->GetText().empty()) {
                printer(*it->second, output);
            }
        }
        output << '\n';
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    Print(output, [](const Cell& cell, std::ostream& out) {
        std::visit([&out](const auto& value) { out << value; }, cell.GetValue());
    });
}

void Sheet::PrintTexts(std::ostream& output) const {
    Print(output, [](const Cell& cell, std::ostream& out) {
        out << cell.GetText();
    });
}

const Cell* Sheet::GetCellPtr(Position pos) const {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");

    const auto cell = cells_.find(pos);
    if (cell == cells_.end()) {
        return nullptr;
    }

    return cells_.at(pos).get();
}

Cell* Sheet::GetCellPtr(Position pos) {
    return const_cast<Cell*>(
        static_cast<const Sheet&>(*this).GetCellPtr(pos));
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}