#include "FileSorts.h"

#include "utils/StringUtil.h"
#include "Settings.h"
#include "Log.h"

namespace FileSorts
{

	const FileData::SortType typesArr[] = {

		// Nombre
		FileData::SortType(&compareName, true,  "name, ascending"),
		FileData::SortType(&compareName, false, "name, descending"),

		// Rating
		FileData::SortType(&compareRating, true,  "rating, ascending"),
		FileData::SortType(&compareRating, false, "rating, descending"),

		// Veces jugado
		FileData::SortType(&compareTimesPlayed, true,  "times played, ascending"),
		FileData::SortType(&compareTimesPlayed, false, "times played, descending"),

		// Última vez jugado
		FileData::SortType(&compareLastPlayed, true,  "last played, ascending"),
		FileData::SortType(&compareLastPlayed, false, "last played, descending"),

		// Jugadores
		FileData::SortType(&compareNumPlayers, true,  "number players, ascending"),
		FileData::SortType(&compareNumPlayers, false, "number players, descending"),

		// Fecha de lanzamiento
		FileData::SortType(&compareReleaseDate, true,  "release date, ascending"),
		FileData::SortType(&compareReleaseDate, false, "release date, descending"),

		// Género
		FileData::SortType(&compareGenre, true,  "genre, ascending"),
		FileData::SortType(&compareGenre, false, "genre, descending"),

		// Desarrollador
		FileData::SortType(&compareDeveloper, true,  "developer, ascending"),
		FileData::SortType(&compareDeveloper, false, "developer, descending"),

		// Distribuidor
		FileData::SortType(&comparePublisher, true,  "publisher, ascending"),
		FileData::SortType(&comparePublisher, false, "publisher, descending"),

		// Sistema
		FileData::SortType(&compareSystem, true,  "system, ascending"),
		FileData::SortType(&compareSystem, false, "system, descending")
	};

	const std::vector<FileData::SortType> SortTypes(typesArr, typesArr + sizeof(typesArr)/sizeof(typesArr[0]));

	//returns if file1 should come before file2
	bool compareName(const FileData* file1, const FileData* file2)
	{
		std::string name1 = Utils::String::toUpper(file1->metadata.get("sortname"));
		std::string name2 = Utils::String::toUpper(file2->metadata.get("sortname"));

		if(name1.empty())
			name1 = Utils::String::toUpper(file1->metadata.get("name"));

		if(name2.empty())
			name2 = Utils::String::toUpper(file2->metadata.get("name"));

		ignoreLeadingArticles(name1, name2);

		return name1.compare(name2) < 0;
	}

	bool compareRating(const FileData* file1, const FileData* file2)
	{
		return file1->metadata.getFloat("rating") < file2->metadata.getFloat("rating");
	}

	bool compareTimesPlayed(const FileData* file1, const FileData* file2)
	{
		if(file1->metadata.getType() == GAME_METADATA && file2->metadata.getType() == GAME_METADATA)
		{
			return file1->metadata.getInt("playcount") < file2->metadata.getInt("playcount");
		}

		return false;
	}

	bool compareLastPlayed(const FileData* file1, const FileData* file2)
	{
		return file1->metadata.get("lastplayed") < file2->metadata.get("lastplayed");
	}

	bool compareNumPlayers(const FileData* file1, const FileData* file2)
	{
		return file1->metadata.getInt("players") < file2->metadata.getInt("players");
	}

	bool compareReleaseDate(const FileData* file1, const FileData* file2)
	{
		return file1->metadata.get("releasedate") < file2->metadata.get("releasedate");
	}

	bool compareGenre(const FileData* file1, const FileData* file2)
	{
		std::string genre1 = Utils::String::toUpper(file1->metadata.get("genre"));
		std::string genre2 = Utils::String::toUpper(file2->metadata.get("genre"));

		return genre1.compare(genre2) < 0;
	}

	bool compareDeveloper(const FileData* file1, const FileData* file2)
	{
		std::string dev1 = Utils::String::toUpper(file1->metadata.get("developer"));
		std::string dev2 = Utils::String::toUpper(file2->metadata.get("developer"));

		return dev1.compare(dev2) < 0;
	}

	bool comparePublisher(const FileData* file1, const FileData* file2)
	{
		std::string pub1 = Utils::String::toUpper(file1->metadata.get("publisher"));
		std::string pub2 = Utils::String::toUpper(file2->metadata.get("publisher"));

		return pub1.compare(pub2) < 0;
	}

	bool compareSystem(const FileData* file1, const FileData* file2)
	{
		std::string sys1 = Utils::String::toUpper(file1->getSystemName());
		std::string sys2 = Utils::String::toUpper(file2->getSystemName());

		return sys1.compare(sys2) < 0;
	}

	// Ignore artículos iniciales (The, El, La, etc.)
	void ignoreLeadingArticles(std::string &name1, std::string &name2)
	{
		if (Settings::getInstance()->getBool("IgnoreLeadingArticles"))
		{
			std::vector<std::string> articles =
				Utils::String::delimitedStringToVector(
					Settings::getInstance()->getString("LeadingArticles"), ",");

			for (auto it = articles.begin(); it != articles.end(); it++)
			{
				if (Utils::String::startsWith(name1, Utils::String::toUpper(*it) + " "))
					name1 = Utils::String::replace(name1, Utils::String::toUpper(*it) + " ", "");

				if (Utils::String::startsWith(name2, Utils::String::toUpper(*it) + " "))
					name2 = Utils::String::replace(name2, Utils::String::toUpper(*it) + " ", "");
			}
		}
	}

};
