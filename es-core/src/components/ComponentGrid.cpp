// ComponentGrid.cpp
#include "components/ComponentGrid.h"

#include "Settings.h"

using namespace GridFlags;

ComponentGrid::ComponentGrid(Window* window, const Vector2i& gridDimensions)
	: GuiComponent(window)
	, mGridSize(gridDimensions)
	, mCursor(0, 0)
	, mColWidths(nullptr)
	, mRowHeights(nullptr)
{
	assert(gridDimensions.x() > 0 && gridDimensions.y() > 0);

	mCells.reserve(gridDimensions.x() * gridDimensions.y());

	mColWidths = new float[gridDimensions.x()];
	mRowHeights = new float[gridDimensions.y()];

	for (int x = 0; x < gridDimensions.x(); x++)
		mColWidths[x] = 0.0f;
	for (int y = 0; y < gridDimensions.y(); y++)
		mRowHeights[y] = 0.0f;
}

ComponentGrid::~ComponentGrid()
{
	delete[] mRowHeights;
	delete[] mColWidths;
}

float ComponentGrid::getColWidth(int col)
{
	assert(col >= 0 && col < mGridSize.x());

	if (mColWidths[col] != 0.0f)
		return mColWidths[col] * mSize.x();

	float freeWidthPerc = 1.0f;
	int between = 0;

	for (int x = 0; x < mGridSize.x(); x++)
	{
		freeWidthPerc -= mColWidths[x];
		if (mColWidths[x] == 0.0f)
			++between;
	}

	if (between <= 0)
		return 0.0f;

	return (freeWidthPerc * mSize.x()) / static_cast<float>(between);
}

float ComponentGrid::getRowHeight(int row)
{
	assert(row >= 0 && row < mGridSize.y());

	if (mRowHeights[row] != 0.0f)
		return mRowHeights[row] * mSize.y();

	float freeHeightPerc = 1.0f;
	int between = 0;

	for (int y = 0; y < mGridSize.y(); y++)
	{
		freeHeightPerc -= mRowHeights[y];
		if (mRowHeights[y] == 0.0f)
			++between;
	}

	if (between <= 0)
		return 0.0f;

	return (freeHeightPerc * mSize.y()) / static_cast<float>(between);
}

void ComponentGrid::setColWidthPerc(int col, float width, bool update)
{
	assert(width >= 0.0f && width <= 1.0f);
	assert(col >= 0 && col < mGridSize.x());

	mColWidths[col] = width;

	if (update)
		onSizeChanged();
}

void ComponentGrid::setRowHeightPerc(int row, float height, bool update)
{
	assert(height >= 0.0f && height <= 1.0f);
	assert(row >= 0 && row < mGridSize.y());

	mRowHeights[row] = height;

	if (update)
		onSizeChanged();
}

void ComponentGrid::setEntry(const std::shared_ptr<GuiComponent>& comp, const Vector2i& pos,
							 bool canFocus, bool resize, const Vector2i& size,
							 unsigned int border, GridFlags::UpdateType updateType)
{
	assert(pos.x() >= 0 && pos.x() < mGridSize.x() && pos.y() >= 0 && pos.y() < mGridSize.y());
	assert(comp != nullptr);
	assert(comp->getParent() == NULL);

	GridEntry entry(pos, size, comp, canFocus, resize, updateType, border);
	mCells.push_back(entry);

	addChild(comp.get());

	if (!cursorValid() && canFocus)
	{
		Vector2i origCursor = mCursor;
		mCursor = pos;
		onCursorMoved(origCursor, mCursor);
	}

	updateCellComponent(mCells.back());
	updateSeparators();
}

bool ComponentGrid::removeEntry(const std::shared_ptr<GuiComponent>& comp)
{
	for (auto it = mCells.cbegin(); it != mCells.cend(); ++it)
	{
		if (it->component == comp)
		{
			removeChild(comp.get());
			mCells.erase(it);
			return true;
		}
	}
	return false;
}

void ComponentGrid::updateCellComponent(const GridEntry& cell)
{
	// Calcular rectángulo de celda (en pixeles)
	Vector2f cellSize(0.0f, 0.0f);
	for (int x = cell.pos.x(); x < cell.pos.x() + cell.dim.x(); x++)
		cellSize[0] += getColWidth(x);
	for (int y = cell.pos.y(); y < cell.pos.y() + cell.dim.y(); y++)
		cellSize[1] += getRowHeight(y);

	// Resize (solo si cambió)
	if (cell.resize)
	{
		if (cell.component->getSize() != cellSize)
			cell.component->setSize(cellSize);
	}

	// Top-left del área
	Vector3f pos(0.0f, 0.0f, 0.0f);
	for (int x = 0; x < cell.pos.x(); x++)
		pos[0] += getColWidth(x);
	for (int y = 0; y < cell.pos.y(); y++)
		pos[1] += getRowHeight(y);

	// Centrar dentro de su celda
	const Vector2f compSize = cell.component->getSize();
	pos[0] = pos.x() + (cellSize.x() - compSize.x()) / 2.0f;
	pos[1] = pos.y() + (cellSize.y() - compSize.y()) / 2.0f;

	cell.component->setPosition(pos);
}

void ComponentGrid::updateSeparators()
{
	mLines.clear();

	const unsigned int color = Renderer::convertColor(0xC6C7C6FF);
	const bool drawAll = Settings::getInstance()->getBool("DebugGrid");

	Vector2f pos;
	Vector2f size;

	for (auto it = mCells.cbegin(); it != mCells.cend(); ++it)
	{
		if (!it->border && !drawAll)
			continue;

		pos = Vector2f(0.0f, 0.0f);
		size = Vector2f(0.0f, 0.0f);

		for (int x = 0; x < it->pos.x(); x++)
			pos[0] += getColWidth(x);
		for (int y = 0; y < it->pos.y(); y++)
			pos[1] += getRowHeight(y);

		for (int x = it->pos.x(); x < it->pos.x() + it->dim.x(); x++)
			size[0] += getColWidth(x);
		for (int y = it->pos.y(); y < it->pos.y() + it->dim.y(); y++)
			size[1] += getRowHeight(y);

		if (it->border & BORDER_TOP || drawAll)
		{
			mLines.push_back({ { pos.x(),            pos.y()            }, { 0.0f, 0.0f }, color });
			mLines.push_back({ { pos.x() + size.x(), pos.y()            }, { 0.0f, 0.0f }, color });
		}
		if (it->border & BORDER_BOTTOM || drawAll)
		{
			mLines.push_back({ { pos.x(),            pos.y() + size.y() }, { 0.0f, 0.0f }, color });
			mLines.push_back({ { pos.x() + size.x(), mLines.back().pos.y() }, { 0.0f, 0.0f }, color });
		}
		if (it->border & BORDER_LEFT || drawAll)
		{
			mLines.push_back({ { pos.x(), pos.y()            }, { 0.0f, 0.0f }, color });
			mLines.push_back({ { pos.x(), pos.y() + size.y() }, { 0.0f, 0.0f }, color });
		}
		if (it->border & BORDER_RIGHT || drawAll)
		{
			mLines.push_back({ { pos.x() + size.x(), pos.y()            }, { 0.0f, 0.0f }, color });
			mLines.push_back({ { mLines.back().pos.x(), pos.y() + size.y() }, { 0.0f, 0.0f }, color });
		}
	}
}

void ComponentGrid::onSizeChanged()
{
	for (auto it = mCells.cbegin(); it != mCells.cend(); ++it)
		updateCellComponent(*it);

	updateSeparators();
}

const ComponentGrid::GridEntry* ComponentGrid::getCellAt(int x, int y) const
{
	assert(x >= 0 && x < mGridSize.x() && y >= 0 && y < mGridSize.y());

	for (auto it = mCells.cbegin(); it != mCells.cend(); ++it)
	{
		const int xmin = it->pos.x();
		const int xmax = xmin + it->dim.x();
		const int ymin = it->pos.y();
		const int ymax = ymin + it->dim.y();

		if (x >= xmin && y >= ymin && x < xmax && y < ymax)
			return &(*it);
	}

	return NULL;
}

bool ComponentGrid::input(InputConfig* config, Input input)
{
	const GridEntry* cursorEntry = getCellAt(mCursor);
	if (cursorEntry && cursorEntry->component->input(config, input))
		return true;

	if (!input.value)
		return false;

	bool withinBoundary = false;

	if (config->isMappedLike("down", input))
		withinBoundary = moveCursor(Vector2i(0, 1));
	else if (config->isMappedLike("up", input))
		withinBoundary = moveCursor(Vector2i(0, -1));
	else if (config->isMappedLike("left", input))
		withinBoundary = moveCursor(Vector2i(-1, 0));
	else if (config->isMappedLike("right", input))
		withinBoundary = moveCursor(Vector2i(1, 0));

	// ✅ Nuevo: si no se pudo mover porque chocaste un borde, delegar al callback
	if (!withinBoundary && mPastBoundaryCallback)
		return mPastBoundaryCallback(config, input);

	return withinBoundary;
}

void ComponentGrid::resetCursor()
{
	if (!mCells.size())
		return;

	for (auto it = mCells.cbegin(); it != mCells.cend(); ++it)
	{
		if (it->canFocus)
		{
			Vector2i origCursor = mCursor;
			mCursor = it->pos;
			onCursorMoved(origCursor, mCursor);
			break;
		}
	}
}

bool ComponentGrid::moveCursor(Vector2i dir)
{
	assert(dir.x() || dir.y());

	const Vector2i origCursor = mCursor;

	const GridEntry* currentCursorEntry = getCellAt(mCursor);
	if (!currentCursorEntry)
		return false;

	Vector2i searchAxis(dir.x() == 0, dir.y() == 0);

	// ✅ ES-DE style: manejar entradas multi-celda
	if (currentCursorEntry->dim.x() > 1)
	{
		if (dir.x() > 0 && mCursor.x() != currentCursorEntry->pos.x() + currentCursorEntry->dim.x() - 1)
			dir[0] = currentCursorEntry->dim.x() - (mCursor.x() - currentCursorEntry->pos.x());
		else if (dir.x() < 0 && mCursor.x() != currentCursorEntry->pos.x())
			dir[0] = -(mCursor.x() - currentCursorEntry->pos.x() + 1);
	}

	if (currentCursorEntry->dim.y() > 1)
	{
		if (dir.y() > 0 && mCursor.y() != currentCursorEntry->pos.y() + currentCursorEntry->dim.y() - 1)
			dir[1] = currentCursorEntry->dim.y() - (mCursor.y() - currentCursorEntry->pos.y());
		else if (dir.y() < 0 && mCursor.y() != currentCursorEntry->pos.y())
			dir[1] = -(mCursor.y() - currentCursorEntry->pos.y() + 1);
	}

	while (mCursor.x() >= 0 && mCursor.y() >= 0 &&
		   mCursor.x() < mGridSize.x() && mCursor.y() < mGridSize.y())
	{
		mCursor = mCursor + dir;
		Vector2i curDirPos = mCursor;

		const GridEntry* cursorEntry = nullptr;

		// Barrido searchAxis +
		while (mCursor.x() < mGridSize.x() && mCursor.y() < mGridSize.y() &&
			   mCursor.x() >= 0 && mCursor.y() >= 0)
		{
			cursorEntry = getCellAt(mCursor);

			if (cursorEntry != nullptr)
			{
				// Evitar “pisar” multi-celda al buscar hacia atrás
				const GridEntry* origCell = getCellAt(origCursor);
				if (origCell)
				{
					if (dir.x() < 0 && cursorEntry->dim.x() > 1)
						mCursor[0] = origCell->pos.x() - cursorEntry->dim.x();
					if (dir.y() < 0 && cursorEntry->dim.y() > 1)
						mCursor[1] = origCell->pos.y() - cursorEntry->dim.y();
				}

				if (cursorEntry->canFocus && cursorEntry != currentCursorEntry)
				{
					onCursorMoved(origCursor, mCursor);
					return true;
				}
			}

			mCursor += searchAxis;
		}

		// Barrido searchAxis -
		mCursor = curDirPos;
		while (mCursor.x() >= 0 && mCursor.y() >= 0 &&
			   mCursor.x() < mGridSize.x() && mCursor.y() < mGridSize.y())
		{
			cursorEntry = getCellAt(mCursor);

			if (cursorEntry && cursorEntry->canFocus && cursorEntry != currentCursorEntry)
			{
				onCursorMoved(origCursor, mCursor);
				return true;
			}

			mCursor -= searchAxis;
		}

		mCursor = curDirPos;
	}

	mCursor = origCursor;
	return false;
}

void ComponentGrid::onFocusLost()
{
	const GridEntry* cursorEntry = getCellAt(mCursor);
	if (cursorEntry)
		cursorEntry->component->onFocusLost();
}

void ComponentGrid::onFocusGained()
{
	const GridEntry* cursorEntry = getCellAt(mCursor);
	if (cursorEntry)
		cursorEntry->component->onFocusGained();
}

bool ComponentGrid::cursorValid()
{
	const GridEntry* e = getCellAt(mCursor);
	return (e != NULL && e->canFocus);
}

void ComponentGrid::update(int deltaTime)
{
	const GridEntry* cursorEntry = getCellAt(mCursor);

	for (auto it = mCells.cbegin(); it != mCells.cend(); ++it)
	{
		if (it->updateType == UPDATE_ALWAYS ||
			(it->updateType == UPDATE_WHEN_SELECTED && cursorEntry == &(*it)))
		{
			it->component->update(deltaTime);
		}
	}
}

void ComponentGrid::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();

	renderChildren(trans);

	if (mLines.size())
	{
		Renderer::setMatrix(trans);
		Renderer::bindTexture(0);
		Renderer::drawLines(&mLines[0], (int)mLines.size());
	}
}

void ComponentGrid::textInput(const char* text)
{
	const GridEntry* selectedEntry = getCellAt(mCursor);
	if (selectedEntry != NULL && selectedEntry->canFocus)
		selectedEntry->component->textInput(text);
}

void ComponentGrid::onCursorMoved(Vector2i from, Vector2i to)
{
	const GridEntry* cell = getCellAt(from);
	if (cell)
		cell->component->onFocusLost();

	cell = getCellAt(to);
	if (cell)
		cell->component->onFocusGained();

	updateHelpPrompts();
}

void ComponentGrid::setCursorTo(const std::shared_ptr<GuiComponent>& comp)
{
	for (auto it = mCells.cbegin(); it != mCells.cend(); ++it)
	{
		if (it->component == comp)
		{
			Vector2i oldCursor = mCursor;
			mCursor = it->pos;
			onCursorMoved(oldCursor, mCursor);
			return;
		}
	}
	assert(false);
}

std::vector<HelpPrompt> ComponentGrid::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	const GridEntry* e = getCellAt(mCursor);
	if (e)
		prompts = e->component->getHelpPrompts();
	else
		return prompts;

	bool canScrollVert = false;
	bool canScrollHoriz = false;

	// ✅ ES-DE style: solo si existe foco real arriba/abajo/izq/der
	if (mGridSize.y() > 1 && e->dim.y() < mGridSize.y())
	{
		// arriba
		if (e->pos.y() - e->dim.y() >= 0)
		{
			const GridEntry* cell = getCellAt(Vector2i(e->pos.x(), e->pos.y() - e->dim.y()));
			if (cell && cell->canFocus)
				canScrollVert = true;
		}
		// abajo
		if (e->pos.y() + e->dim.y() < mGridSize.y())
		{
			const GridEntry* cell = getCellAt(Vector2i(e->pos.x(), e->pos.y() + e->dim.y()));
			if (cell && cell->canFocus)
				canScrollVert = true;
		}
	}

	if (mGridSize.x() > 1 && e->dim.x() < mGridSize.x())
	{
		// izquierda
		if (e->pos.x() - e->dim.x() >= 0)
		{
			const GridEntry* cell = getCellAt(Vector2i(e->pos.x() - e->dim.x(), e->pos.y()));
			if (cell && cell->canFocus)
				canScrollHoriz = true;
		}
		// derecha
		if (e->pos.x() + e->dim.x() < mGridSize.x())
		{
			const GridEntry* cell = getCellAt(Vector2i(e->pos.x() + e->dim.x(), e->pos.y()));
			if (cell && cell->canFocus)
				canScrollHoriz = true;
		}
	}

	// Mantener compatibilidad con prompts existentes del componente
	if (!prompts.empty() && prompts.back() == HelpPrompt("up/down", "choose"))
	{
		canScrollVert = true;
		if (canScrollHoriz && canScrollVert)
			prompts.pop_back();
	}
	else if (!prompts.empty() && prompts.back() == HelpPrompt("left/right", "choose"))
	{
		canScrollHoriz = true;
		if (canScrollHoriz && canScrollVert)
			prompts.pop_back();
	}

	if (canScrollHoriz && canScrollVert)
		prompts.push_back(HelpPrompt("up/down/left/right", "choose"));
	else if (canScrollHoriz)
		prompts.push_back(HelpPrompt("left/right", "choose"));
	else if (canScrollVert)
		prompts.push_back(HelpPrompt("up/down", "choose"));

	return prompts;
}
