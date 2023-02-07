#include "ImdbMovie.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QTextDocument>
#include <QWidget>

#include "scrapers/imdb/ImdbReferencePage.h"
#include "scrapers/movie/imdb/ImdbMovieScraper.h"
#include "scrapers/movie/imdb/ImdbMovieSearchJob.h"
#include "ui/main/MainWindow.h"

namespace mediaelch {
namespace scraper {

ImdbMovie::ImdbMovie(QObject* parent) : MovieScraper(parent), m_scrapeJob(m_api, {}, false, nullptr)
{
    m_meta.identifier = ID;
    m_meta.name = "IMDb";
    m_meta.description = tr("IMDb is the world's most popular and authoritative source for movie, TV "
                            "and celebrity content, designed to help fans explore the world of movies "
                            "and shows and decide what to watch.");
    m_meta.website = "https://www.imdb.com/whats-on-tv/";
    m_meta.termsOfService = "https://www.imdb.com/conditions";
    m_meta.privacyPolicy = "https://www.imdb.com/privacy";
    m_meta.help = "https://help.imdb.com";
    m_meta.supportedDetails = {MovieScraperInfo::Title,
        MovieScraperInfo::Director,
        MovieScraperInfo::Writer,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Tags,
        MovieScraperInfo::Released,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Tagline,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Poster};
    m_meta.supportedLanguages = {"en"};
    m_meta.defaultLocale = "en";
    m_meta.isAdult = false;

    m_settingsWidget = new QWidget;
    m_loadAllTagsWidget = new QCheckBox(tr("Load all tags"), m_settingsWidget);
    auto* layout = new QGridLayout(m_settingsWidget);
    layout->addWidget(m_loadAllTagsWidget, 0, 0);
    layout->setContentsMargins(12, 0, 12, 12);
    m_settingsWidget->setLayout(layout);
}

ImdbMovie::~ImdbMovie()
{
    if (m_settingsWidget != nullptr && m_settingsWidget->parent() == nullptr) {
        // We set MainWindow::instance() as this Widget's parent.
        // But at construction time, the instance is not setup, yet.
        // See settingsWidget()
        delete m_settingsWidget;
    }
}

const MovieScraper::ScraperMeta& ImdbMovie::meta() const
{
    return m_meta;
}

void ImdbMovie::initialize()
{
    // no-op
    // IMDb requires no initialization.
}

bool ImdbMovie::isInitialized() const
{
    // IMDb requires no initialization.
    return true;
}

bool ImdbMovie::hasSettings() const
{
    return true;
}

MovieSearchJob* ImdbMovie::search(MovieSearchJob::Config config)
{
    return new ImdbMovieSearchJob(m_api, std::move(config), this);
}

QWidget* ImdbMovie::settingsWidget()
{
    if (m_settingsWidget->parent() == nullptr) {
        m_settingsWidget->setParent(MainWindow::instance());
    }
    return m_settingsWidget;
}

void ImdbMovie::loadSettings(ScraperSettings& settings)
{
    m_loadAllTags = settings.valueBool("LoadAllTags", false);
    m_loadAllTagsWidget->setChecked(m_loadAllTags);
}

void ImdbMovie::saveSettings(ScraperSettings& settings)
{
    m_loadAllTags = m_loadAllTagsWidget->isChecked();
    settings.setBool("LoadAllTags", m_loadAllTags);
}

QSet<MovieScraperInfo> ImdbMovie::scraperNativelySupports()
{
    return m_meta.supportedDetails;
}

void ImdbMovie::changeLanguage(mediaelch::Locale /*locale*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

void ImdbMovie::loadData(QHash<MovieScraper*, mediaelch::scraper::MovieIdentifier> ids,
    Movie* movie,
    QSet<MovieScraperInfo> infos)
{
    if (movie == nullptr) {
        return;
    }
    ImdbId imdbId(ids.values().first().str());
    auto* loader =
        new mediaelch::scraper::ImdbMovieLoader(m_api, *this, imdbId, *movie, std::move(infos), m_loadAllTags, this);
    connect(loader, &ImdbMovieLoader::sigLoadDone, this, &ImdbMovie::onLoadDone);
    loader->load();
}

void ImdbMovie::onLoadDone(Movie& movie, mediaelch::scraper::ImdbMovieLoader* loader)
{
    loader->deleteLater();
    movie.controller()->scraperLoadDone(this, {}); // TODO: Error
}

void ImdbMovie::parseAndAssignInfos(const QString& html, Movie* movie, QSet<MovieScraperInfo> infos) const
{
    m_scrapeJob.parseAndAssignInfos(html, movie, infos);
}


} // namespace scraper
} // namespace mediaelch
