#include "Graph.h"
#include "Line.h"

namespace ui {

	Graph::Graph(const std::string& id)
		: UIObject(id)
		, range({ 0, 100 }, { 0, 100 })
		, step(1)
	{
		m_backgrownd.setPosition(0, 0);
		m_backgrownd.setSize({ 100, 100 });
		m_backgrownd.setFillColor(sf::Color::White);
	}

	void Graph::SetPosition(const sf::Vector2f& pos)
	{
		m_backgrownd.setPosition(pos);
	}

	void Graph::SetSize(const sf::Vector2f& size)
	{
		m_backgrownd.setSize(size);
	}

	void Graph::SetRange(const sf::Vector2<sf::Vector2f>& range)
	{
		this->range = range;
	}

	void Graph::SetStep(const float& step)
	{
		this->step = step;
	}

	void Graph::AddData(const float& value)
	{
		m_values.push_back(value);

		if ((m_values.size() - 1) * step > range.x.y)
		{
			m_values.erase(m_values.begin());
		}
	}

	void Graph::Draw(sf::RenderWindow& window)
	{
		window.draw(m_backgrownd);

		sf::Vector2f pos = m_backgrownd.getPosition();

		for (uint i = 1; i < m_values.size(); i++)
		{
			float x1 = map((i - 1) * step, range.x.x, range.x.y, 0, m_backgrownd.getSize().x);
			float y1 = map(m_values[i - 1], range.y.x, range.y.y, 0, m_backgrownd.getSize().y);
			
			float x2 = map(i * step, range.x.x, range.x.y, 0, m_backgrownd.getSize().x);
			float y2 = map(m_values[i], range.y.x, range.y.y, 0, m_backgrownd.getSize().y);

			ui::Line l("", sf::Vector2f(x1, y1) + pos, sf::Vector2f(x2, y2) + pos);
			l.SetColor(sf::Color::Black);
			l.SetWidth(2);

			l.Draw(window);
		}
	}

	Graph::~Graph()
	{
	}

}