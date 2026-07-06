#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <limits>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct WeatherData
{
    double latitude = 0.0;
    double longitude = 0.0;
    std::string time;
    double temperature = 0.0;
    int humidity = 0;
    double windSpeed = 0.0;
    int weatherCode = -1;
    std::string temperatureUnit;
    std::string humidityUnit;
    std::string windSpeedUnit;
};

struct Location
{
    std::string name;
    double latitude;
    double longitude;
};

void displayMenu()
{
    std::cout << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "          Weather API Client" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "1. Get Weather for Default Location" << std::endl;
    std::cout << "2. Get Weather by Coordinates" << std::endl;
    std::cout << "3. Get Weather for Favorite Location" << std::endl;
    std::cout << "4. Exit" << std::endl;
    std::cout << "Please choose an option: ";
}

bool readInteger(int& value)
{
    std::cin >> value;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Invalid input. Please enter a number." << std::endl;
        return false;
    }

    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    return true;
}

bool readDouble(double& value)
{
    std::cin >> value;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cout << "Invalid input. Please enter a valid number." << std::endl;
        return false;
    }

    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    return true;
}

bool isValidLatitude(double latitude)
{
    return latitude >= -90.0 && latitude <= 90.0;
}

bool isValidLongitude(double longitude)
{
    return longitude >= -180.0 && longitude <= 180.0;
}

std::string buildWeatherUrl(double latitude, double longitude)
{
    std::ostringstream url;

    url << "https://api.open-meteo.com/v1/forecast?"
        << "latitude=" << latitude
        << "&longitude=" << longitude
        << "&current=temperature_2m,relative_humidity_2m,wind_speed_10m,weather_code"
        << "&timezone=auto";

    return url.str();
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userData)
{
    size_t totalSize = size * nmemb;

    std::string* response = static_cast<std::string*>(userData);
    response->append(static_cast<char*>(contents), totalSize);

    return totalSize;
}

bool httpGet(const std::string& url, std::string& response)
{
    CURL* curl = curl_easy_init();

    if (!curl)
    {
        std::cout << "Error: Could not initialize CURL." << std::endl;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Chapter14_WeatherApiClient/1.0");

    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        std::cout << "CURL error: " << curl_easy_strerror(result) << std::endl;
        curl_easy_cleanup(curl);
        return false;
    }

    long httpStatusCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatusCode);

    curl_easy_cleanup(curl);

    if (httpStatusCode != 200)
    {
        std::cout << "HTTP error. Status code: " << httpStatusCode << std::endl;
        return false;
    }

    if (response.empty())
    {
        std::cout << "Error: The API returned an empty response." << std::endl;
        return false;
    }

    return true;
}

std::string getWeatherDescription(int code)
{
    if (code == 0)
    {
        return "Clear sky";
    }
    else if (code == 1 || code == 2 || code == 3)
    {
        return "Mainly clear, partly cloudy, or overcast";
    }
    else if (code == 45 || code == 48)
    {
        return "Fog";
    }
    else if (code >= 51 && code <= 57)
    {
        return "Drizzle";
    }
    else if (code >= 61 && code <= 67)
    {
        return "Rain";
    }
    else if (code >= 71 && code <= 77)
    {
        return "Snow";
    }
    else if (code >= 80 && code <= 82)
    {
        return "Rain showers";
    }
    else if (code >= 95 && code <= 99)
    {
        return "Thunderstorm";
    }

    return "Unknown weather condition";
}

bool parseWeatherJson(const std::string& response, WeatherData& weather)
{
    try
    {
        json data = json::parse(response);

        if (!data.contains("current"))
        {
            std::cout << "Error: The response does not contain current weather data." << std::endl;
            return false;
        }

        weather.latitude = data.value("latitude", 0.0);
        weather.longitude = data.value("longitude", 0.0);

        json current = data["current"];
        json units = data.value("current_units", json::object());

        weather.time = current.value("time", "");
        weather.temperature = current.value("temperature_2m", 0.0);
        weather.humidity = current.value("relative_humidity_2m", 0);
        weather.windSpeed = current.value("wind_speed_10m", 0.0);
        weather.weatherCode = current.value("weather_code", -1);

        weather.temperatureUnit = units.value("temperature_2m", "C");
        weather.humidityUnit = units.value("relative_humidity_2m", "%");
        weather.windSpeedUnit = units.value("wind_speed_10m", "km/h");

        return true;
    }
    catch (const std::exception& ex)
    {
        std::cout << "JSON parsing error: " << ex.what() << std::endl;
        return false;
    }
}

void displayWeather(const std::string& locationName, const WeatherData& weather)
{
    std::cout << std::endl;
    std::cout << "========== Current Weather ==========" << std::endl;
    std::cout << "Location          : " << locationName << std::endl;
    std::cout << "Latitude          : " << weather.latitude << std::endl;
    std::cout << "Longitude         : " << weather.longitude << std::endl;
    std::cout << "Time              : " << weather.time << std::endl;

    std::cout << "Temperature       : "
        << std::fixed << std::setprecision(1)
        << weather.temperature << " "
        << weather.temperatureUnit << std::endl;

    std::cout << "Humidity          : "
        << weather.humidity << " "
        << weather.humidityUnit << std::endl;

    std::cout << "Wind Speed        : "
        << std::fixed << std::setprecision(1)
        << weather.windSpeed << " "
        << weather.windSpeedUnit << std::endl;

    std::cout << "Weather Condition : "
        << getWeatherDescription(weather.weatherCode)
        << std::endl;
}

void getWeather(const std::string& locationName, double latitude, double longitude)
{
    std::string url = buildWeatherUrl(latitude, longitude);
    std::string response;

    std::cout << "Requesting weather data..." << std::endl;

    if (!httpGet(url, response))
    {
        std::cout << "Failed to retrieve weather data." << std::endl;
        return;
    }

    WeatherData weather;

    if (!parseWeatherJson(response, weather))
    {
        std::cout << "Failed to parse weather data." << std::endl;
        return;
    }

    displayWeather(locationName, weather);
}

void getWeatherByCoordinates()
{
    double latitude;
    double longitude;

    std::cout << "Enter latitude: ";

    if (!readDouble(latitude))
    {
        return;
    }

    std::cout << "Enter longitude: ";

    if (!readDouble(longitude))
    {
        return;
    }

    if (!isValidLatitude(latitude))
    {
        std::cout << "Invalid latitude. Latitude must be between -90 and 90." << std::endl;
        return;
    }

    if (!isValidLongitude(longitude))
    {
        std::cout << "Invalid longitude. Longitude must be between -180 and 180." << std::endl;
        return;
    }

    getWeather("Custom Location", latitude, longitude);
}

void getWeatherForFavoriteLocation(const std::vector<Location>& locations)
{
    std::cout << std::endl;
    std::cout << "========== Favorite Locations ==========" << std::endl;

    for (int i = 0; i < static_cast<int>(locations.size()); ++i)
    {
        std::cout << i + 1 << ". " << locations[i].name << std::endl;
    }

    std::cout << "Choose a location: ";

    int choice;

    if (!readInteger(choice))
    {
        return;
    }

    if (choice < 1 || choice > static_cast<int>(locations.size()))
    {
        std::cout << "Invalid location choice." << std::endl;
        return;
    }

    const Location& selectedLocation = locations[choice - 1];

    getWeather(
        selectedLocation.name,
        selectedLocation.latitude,
        selectedLocation.longitude
    );
}

int main()
{
    CURLcode curlInitResult = curl_global_init(CURL_GLOBAL_DEFAULT);

    if (curlInitResult != CURLE_OK)
    {
        std::cout << "Error: Could not initialize libcurl." << std::endl;
        return 1;
    }

    const Location defaultLocation =
    {
        "Kuala Lumpur",
        3.1390,
        101.6869
    };

    std::vector<Location> favoriteLocations =
    {
        {"Kuala Lumpur", 3.1390, 101.6869},
        {"Singapore", 1.3521, 103.8198},
        {"Bangkok", 13.7563, 100.5018},
        {"Tokyo", 35.6762, 139.6503},
        {"London", 51.5072, -0.1276}
    };

    int choice;

    while (true)
    {
        displayMenu();

        if (!readInteger(choice))
        {
            continue;
        }

        switch (choice)
        {
        case 1:
            getWeather(
                defaultLocation.name,
                defaultLocation.latitude,
                defaultLocation.longitude
            );
            break;

        case 2:
            getWeatherByCoordinates();
            break;

        case 3:
            getWeatherForFavoriteLocation(favoriteLocations);
            break;

        case 4:
            curl_global_cleanup();
            std::cout << "Thank you for using the Weather API Client." << std::endl;
            return 0;

        default:
            std::cout << "Invalid option. Please choose again." << std::endl;
        }
    }
}