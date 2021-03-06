#include <iostream>
#include "..\Minesweeper\Minesweeper.h"
#include <UITools/UITools.h>
#include <sstream>
#include <iomanip>

using uint = unsigned int;

std::ostream& operator<<(std::ostream& os, const Minesweeper::State& state)
{
	switch (state)
	{
	case Minesweeper::Start:
		os << "Start";
		break;

	case Minesweeper::Playing:
		os << "Playing";
		break;

	case Minesweeper::Win:
		os << "Win";
		break;

	case Minesweeper::Lost:
		os << "Lost";
		break;

	case Minesweeper::Disconnect:
		os << "Disconnect";
		break;

	default:
		os << "Unknown";
		break;
	}
	return os;
}


struct PlayerData
{
	Minesweeper::State state = Minesweeper::Playing;
	sf::Time time = sf::Time::Zero;
	float percentage = 0;
	char name[128];
};


bool ComparePlayers(const PlayerData& a, const PlayerData& b)
{
	if (a.state == Minesweeper::Win)
	{
		if (b.state != Minesweeper::Win)
			return true;

		else
			return a.time < b.time;
	}

	if (a.state == Minesweeper::Lost)
	{
		if (b.state != Minesweeper::Lost)
			return false;

		else
			return a.percentage > b.percentage;
	}

	return a.percentage > b.percentage;
}



int main()
{
#if 0
	std::vector<PlayerData> players;

	players.push_back({ Minesweeper::Playing, sf::seconds(4), 50, "asdf" });
	players.push_back({ Minesweeper::Lost, sf::seconds(2), 23, "qwer" });
	players.push_back({ Minesweeper::Win, sf::seconds(5), 100, "1234", });
	players.push_back({ Minesweeper::Win, sf::seconds(6), 100, "6789" });

	std::sort(players.begin(), players.end(), ComparePlayers);

	for (PlayerData& p : players)
	{
		std::cout << std::string(p.name) << ": " << p.state << " " << p.percentage << "% (" << p.time.asSeconds() << ")" << std::endl;
	}


#else
	std::cout << "Enter board x: ";
	int xSize;
	std::cin >> xSize;

	std::cout << "Enter board y: ";
	int ySize;
	std::cin >> ySize;

	std::cout << "Enter bomb density: ";
	float bombDensity;
	std::cin >> bombDensity;
	

	sf::Vector2i size = { xSize, ySize };
	int nBombs = (int)((float)(size.x * size.y) * bombDensity);
	std::cout << nBombs << std::endl;

	Minesweeper ms(size, nBombs);

	float wSize = 600;
	float sliderSpace = 50;

	sf::Font font;
	font.loadFromFile("basis33.ttf");

	float scale = wSize / size.y;

	sf::Vector2f canvasSize = { size.x * scale, size.y * scale };

	sf::RenderWindow window({ (uint)canvasSize.x, (uint)canvasSize.y + (uint)sliderSpace }, "Minesweeper", sf::Style::Close);


	std::vector<sf::Vertex> grid;
	sf::Color gridColor = { 150, 150, 150 };
	for (float x = scale; x < size.x * scale; x += scale)
	{
		grid.push_back(sf::Vertex({ x, 0 }, gridColor));
		grid.push_back(sf::Vertex({ x, (float)size.y * scale }, gridColor));
	}

	for (float y = scale; y < size.y * scale; y += scale)
	{
		grid.push_back(sf::Vertex({ 0, y }, gridColor));
		grid.push_back(sf::Vertex({ (float)size.x * scale, y }, gridColor));
	}


	std::vector<sf::Vertex> quad;
	for (float y = 0; y < size.y; y++)
	{
		for (float x = 0; x < size.x; x++)
		{
			//sf::Color color = { sf::Uint8(255 * x / size.x), 0, sf::Uint8(255 * y / size.y) };
			sf::Color color = sf::Color::Black;
			quad.push_back(sf::Vertex(sf::Vector2f(x, y) * scale, color));
			quad.push_back(sf::Vertex(sf::Vector2f(x, y + 1) * scale, color));
			quad.push_back(sf::Vertex(sf::Vector2f(x + 1, y + 1) * scale, color));
			quad.push_back(sf::Vertex(sf::Vector2f(x + 1, y) * scale, color));
		}
	}

	std::vector<Minesweeper> historyData;
	historyData.push_back(ms);
	historyData.back().state = Minesweeper::Win;


	ui::Widget ui;

	ui::Slider* historySlider = new ui::Slider("slider", font);
	historySlider->SetPosition(15, canvasSize.y + 15);
	historySlider->SetSize(canvasSize.x - 30, sliderSpace - 30);
	historySlider->ShowValue(false);
	historySlider->SetRange({ 0, 1 });
	historySlider->SetValue(1.f);
	historySlider->SetUpdateFunction([&](ui::UIObject* obj)
	{
		auto self = dynamic_cast<ui::Slider*>(obj);

		if (ms.state != Minesweeper::Win && ms.state != Minesweeper::Lost)
			return;

		int index = (int)ui::map(self->GetValue(), 0.f, 1.f, 0.f, (float)(historyData.size() - 1));

		ms = historyData[index];
	});

	ui.AddObject(historySlider);

	while (window.isOpen())
	{
		ui::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();

			ui.CheckInput(window, e);

			if (e.type == sf::Event::MouseButtonPressed)
			{
				sf::Vector2i pos = (sf::Vector2i)((sf::Vector2f)sf::Mouse::getPosition(window) / scale);

				if (pos.x < 0 || pos.x >= size.x || pos.y < 0 || pos.y >= size.y)
					break;

				bool updated = false;
				if (e.key.code == sf::Mouse::Right)
					updated = ms.Mark(pos);
				
				if (e.key.code == sf::Mouse::Left)
					updated = ms.Discover(pos);
				

				if (updated)
				{
					historyData.push_back(ms);
					historyData.back().state = ms.state == Minesweeper::Lost ? Minesweeper::Lost : Minesweeper::Win;
				}
			}
		}

		window.clear();

		window.draw(&quad[0], quad.size(), sf::Quads);

		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				Minesweeper::Square& curr = ms.board[ms.Get(x, y)];

				sf::Color color = sf::Color::White;

				if (curr.marked)
				{
					if (ms.state == Minesweeper::Win || ms.state == Minesweeper::Lost)
						color = curr.bomb ? sf::Color::Green : sf::Color::Red;

					else
						color = sf::Color::Red;
				}
				else if (curr.bomb)
				{
					if (ms.state == Minesweeper::Lost)
						color = { 255, 220, 0 };
				}
				else if (curr.discovered)
				{
					color = sf::Color::Black;

					if (curr.neighbours > 0)
					{
						sf::Text text(std::to_string(ms.board[ms.Get(x, y)].neighbours), font);
						text.setCharacterSize((uint)scale);
						text.setFillColor(sf::Color::White);

						text.setPosition(x * scale + scale / 4.f, y * scale - scale / 4.f);

						window.draw(text);
					}
				}

				quad[(ms.Get(x, y) * 4) + 0].color = color;
				quad[(ms.Get(x, y) * 4) + 1].color = color;
				quad[(ms.Get(x, y) * 4) + 2].color = color;
				quad[(ms.Get(x, y) * 4) + 3].color = color;
			}
		}

		window.draw(&grid[0], grid.size(), sf::Lines);

		ui.Update(window);
		ui.Draw(window);

		window.display();
	}
#endif
}