#include <utility>
#include <SFML/Graphics.hpp>

#include "menu.hpp"

void gui::Menu::formatEntries()
{
	// First split each entry into lines which fit within the desired bounds
	std::vector<std::vector<std::string>> alignedEntries;
	for(auto entry : entries)
		alignedEntries.push_back(gui::alignString(entry.first, entrySize.x));

	// If laid out from left to right and top to bottom, then the
	// quotient of (number of entries) / (entries per row) will give
	// the number of completed rows, and the remainder will give the
	// number of entries on the partially completed row.
	unsigned int numRows = entries.size() / alignment.x;
	unsigned int partialRow = entries.size() % alignment.x;
	// We now concatenate the first entrySize.y lines of each
	// group of alignment.x entries. As an example if alignment is (3,1)
	// and entrySize is (5, 2) we will transform
	// {
	// 		{"opt1"},
	// 		{"opti2","line2"},
	// 		{"op3", "line2", "line3"}
	// }
	// to
	// {
	// 		{" opt1  ", "       "},
	// 		{" opti2 ", " line2 "},
	// 		{" op3   ", " line2 ", " line3 "}
	// }
	// and then to
	// {
	// 		{" opt1   opti2  op3   "},
	// 		{"        line2  line2 "}
	// }
	// Note that we will also add the correct padding, and will discard any
	// text outside of the bounds (if each entry were a messagebox, they would
	// be on another page).
	for(unsigned int i = 0; i < alignedEntries.size(); ++i)
	{
		// We need an index variable to check for selected elements
		auto& entry = alignedEntries[i];

		// First pad the existing rows
		for(size_t row = 0; row < entry.size(); ++row)
		{
			// Far left and right padding is 1, internal padding is 2,
			// which is equivalent to each entry being surrounded by 1
			// space. Each entry should also have additional right padding
			// so that it's the same length as entrySize.x
			entry[row] = (i == selectedEntry ? std::string(1, selectorCharacter) : " ") +
				entry[row] + std::string(1 + entrySize.x - entry[row].size(), ' ');
		}
		// Now add additional rows made of whitespace, if necessary
		for(size_t row = entry.size(); row < entrySize.y; ++row)
		{
			entry.push_back(std::string(entrySize.x + 2, ' '));
		}
	}
	// Now each entry is padded, they can be combined
	alignedLines.clear();
	// Note row is not a line, but rather a row of entries
	// which may span multiple lines
	for(unsigned int row = 0; row < numRows; ++row)
	{
		for(unsigned int line = 0; line < entrySize.y; ++line)
		{
			std::string alignedLine;
			for(unsigned int entry = 0; entry < alignment.x; ++entry)
			{
				alignedLine += alignedEntries[row * alignment.x + entry][line];
			}
			alignedLines.push_back(alignedLine);
		}
	}
	// Now do the last partial row, if there is one
	if(partialRow > 0)
	{
		for(unsigned int line = 0; line < entrySize.y; ++line)
		{
			std::string alignedLine;
			for(unsigned int entry = 0; entry < partialRow; ++entry)
			{
				alignedLine += alignedEntries[numRows * alignment.x + entry][line];
			}
			alignedLine += std::string((alignment.x-partialRow) * (2+entrySize.x), ' ');
			alignedLines.push_back(alignedLine);
		}
	}
}

void gui::Menu::generateGeometry()
{
	// Width and height of final text, without borders
	unsigned int width = alignment.x * (entrySize.x + 2);
	unsigned int height = alignment.y * entrySize.y;

	// Add top border
	std::string textStr = "\x80" + std::string(width, '\x87') + "\x81\n";

	// Add entry lines, up to the maximum number allowable
	unsigned int bound = alignedLines.size();
	if(bound > height) bound = height;
	for(unsigned int i = 0; i < bound; ++i)
	{
		textStr += "\x86" + alignedLines[i] + "\x84\n";
	}

	// Add padding lines, up to the indented height
	std::string padding(width, ' ');
	for(unsigned int i = bound; i < height; ++i)
	{
		textStr += "\x86" + padding + "\x84\n";
	}

	// Add bottom border
	textStr += "\x83" + std::string(width, '\x85') + "\x82";

	// Finally we can create the gui::Text
	text = gui::Text(textStr, *font);
	text.setColor(textCol);
	text.setBackgroundColor(backgroundCol);
}

void gui::Menu::select(unsigned int index, unsigned char selector)
{
	selectedEntry = index;
	selectorCharacter = selector;
	formatEntries();
	generateGeometry();
}

void gui::Menu::activate(unsigned int index)
{
	entries.at(index).second(index);
}

void gui::Menu::addEntry(const std::string& entry, void (*callback)(int))
{
	entries.push_back(std::make_pair(entry, callback));
	formatEntries();
	generateGeometry();
}

void gui::Menu::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	text.draw(target, states);
}

gui::Menu::Menu(const sf::Vector2u alignment, const sf::Vector2u& entrySize,
		const gui::Font& font, const sf::Color& backgroundCol, const sf::Color& textCol)
{
	this->alignment = alignment;
	this->entrySize = entrySize;
	this->font = &font;
	this->textCol = textCol;
	this->backgroundCol = backgroundCol;
	this->selectedEntry = 0;
	this->selectorCharacter = ' ';
	formatEntries();
	generateGeometry();
}

const sf::Color& gui::Menu::getBackgroundColor() const { return text.getBackgroundColor(); }
const sf::Color& gui::Menu::getColor()			 const { return text.getColor(); }

void gui::Menu::setBackgroundColor(const sf::Color& backgroundCol)
{
	this->backgroundCol = backgroundCol;
	text.setBackgroundColor(backgroundCol);
}

void gui::Menu::setColor(const sf::Color& textCol)
{
	this->textCol = textCol;
	text.setColor(textCol);
}
