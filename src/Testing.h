#include <map>
#include <random>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

struct DebugStat
{
	std::size_t width, height;
	std::size_t noAdj;
	std::size_t initWatched;
	std::size_t maxWatched;
	std::string configName;
};

void addStat(std::vector<DebugStat>& stats, std::string configName,
	std::size_t w, std::size_t h, std::size_t noAdj,
	std::size_t initWatched, std::size_t maxWatched)
{
	stats.push_back(DebugStat{w, h, noAdj, initWatched, maxWatched, configName});
}

void test()
{
	std::vector<DebugStat> defaultStats;
	std::vector<DebugStat> stats;
	std::mt19937_64 gen(std::random_device{}());


	struct Config { std::size_t w, h, mines; std::string name; };
	std::vector<Config> defaultConfigs = {
		{9, 9, 10, "Easy"},
		{16, 16, 40, "Medium"},
		{30, 16, 99, "Hard"}
	};
	constexpr int repetitions = 50;


	for (const auto& cfg : defaultConfigs)
	{
		for (int i = 0; i < repetitions; ++i)
		{
			Board b;
			b.resize({cfg.w, cfg.h});
			b.setMineCount(cfg.mines);
			b.placeMines();

			for (std::size_t i = 0; i < b.getCells().size(); ++i)
			{
				if (!b.getCellAt(i).opened) b.open(i);
			}


			std::size_t noAdj = b.noAdj;
			std::size_t initWatched = b.initWatched;
			std::size_t maxWatched = b.maxWatched;

			addStat(defaultStats, cfg.name, cfg.w, cfg.h, noAdj, initWatched, maxWatched);
		}
	}


	for (std::size_t n = 0; n < 10'000; ++n)
	{
		Board b;
		Vec2s size{};
		while (!b.isSizeValid(size))
		{
			std::uniform_int_distribution<std::size_t> dist(5, 50);
			size = {dist(gen), dist(gen)};
		}
		b.resize(size);
		{
			std::uniform_int_distribution<std::size_t> dist(10, b.getMaxNumberOfMines());
			b.setMineCount(dist(gen));
		}
		b.placeMines();

		for (std::size_t i = 0; i < b.getCells().size(); ++i)
		{
			if (!b.getCellAt(i).opened) b.open(i);
		}

		addStat(stats, "Random", size.x, size.y,
			b.noAdj, b.initWatched, b.maxWatched);
	}


	std::size_t errors = 0;
	double totalRatio = 0;
	double worstRatio = 1;
	std::size_t total = 0;

	std::vector<DebugStat> underEstimations;

	for (const auto& s : stats)
	{
		if (s.initWatched < s.maxWatched)
		{
			++errors;
			underEstimations.push_back(s);
		}

		if (s.maxWatched > 0)
		{
			double ratio = static_cast<double>(s.initWatched) / s.maxWatched;
			totalRatio += ratio;
			++total;
			if (ratio < worstRatio) worstRatio = ratio;
		}
	}

	std::cout << "=== Random ===\n";
	std::cout << "Total: " << total << '\n';
	std::cout << "Errors (init < max): " << errors << " (" << (100.0 * errors / total) << "%)\n";
	std::cout << "Avg ratio: " << (totalRatio / total) << '\n';
	std::cout << "worst ratio: " << worstRatio << "\n\n";

	errors = 0;
	totalRatio = 0;
	worstRatio = 1;
	total = 0;

	for (const auto& s : defaultStats)
	{
		if (s.initWatched < s.maxWatched)
			++errors;

		if (s.maxWatched > 0)
		{
			double ratio = static_cast<double>(s.initWatched) / s.maxWatched;
			totalRatio += ratio;
			++total;
			if (ratio < worstRatio) worstRatio = ratio;
		}
	}

	std::cout << "=== Default Configurations ===\n";
	std::cout << "Total: " << total << '\n';
	std::cout << "Errors (init < max): " << errors << " (" << (100.0 * errors / total) << "%)\n";
	std::cout << "Avg ratio: " << (totalRatio / total) << '\n';
	std::cout << "worst ratio: " << worstRatio << "\n\n";

	std::cout << "=== Important ===\n";

	std::sort(underEstimations.begin(), underEstimations.end(),
		[](const DebugStat& a, const DebugStat& b)
	{
		return (b.maxWatched - b.initWatched) < (a.maxWatched - a.initWatched);
	});

	for (std::size_t i = 0; i < std::min<std::size_t>(20, underEstimations.size()); ++i)
	{
		const auto& s = underEstimations[i];
		std::cout << "Config: " << s.configName
			<< ", Size: " << s.width << 'x' << s.height
			<< ", noAdj: " << s.noAdj
			<< ", init: " << s.initWatched
			<< ", max: " << s.maxWatched
			<< ", ratio: " << static_cast<double>(s.initWatched) / s.maxWatched
			<< '\n';
	}
}
