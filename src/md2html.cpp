#include "md2html.h"

Md2Html::Md2Html(QObject *parent) : QObject(parent)
{

}

static inline int nonOverlapCount(const QString &str, const QChar c = '`') {
    const auto len = str.length();
    int count = 0;
    for (int i = 0; i < len; ++i) {
        if (str[i] == c && i + 2 < len && str[i + 1] == c && str[i + 2] == c) {
            ++count;
            i += 2;
        }
    }
    return count;
}

static void captureHtmlFragment(const MD_CHAR *data, MD_SIZE data_size,
                                void *userData) {
    QByteArray *array = static_cast<QByteArray *>(userData);

    if (data_size > 0) {
        array->append(data, int(data_size));
    }
}

static void highlightCode(QString &str, const QString &type, int cbCount) {
    if (cbCount >= 1) {
        const int firstBlock = str.indexOf(type, 0);
        int currentCbPos = firstBlock;
        for (int i = 0; i < cbCount; ++i) {
            // find endline
            const int endline = str.indexOf(QChar('\n'), currentCbPos);
            const QString lang =
                str.mid(currentCbPos + 3, endline - (currentCbPos + 3));
            // we skip it because it is inline code and not codeBlock
            if (lang.contains(type)) {
                int nextEnd = str.indexOf(type, currentCbPos + 3);
                nextEnd += 3;
                currentCbPos = str.indexOf(type, nextEnd);
                continue;
            }
            // move start pos to after the endline

            currentCbPos = endline + 1;
            // find the codeBlock end
            int next = str.indexOf(type, currentCbPos);
            // extract the codeBlock
            const QStringRef codeBlock =
                str.midRef(currentCbPos, next - currentCbPos);

            QString highlightedCodeBlock;
            if (!(codeBlock.isEmpty() && lang.isEmpty())) {
                const CodeToHtmlConverter c(lang);
                highlightedCodeBlock = c.process(codeBlock);
                // take care of the null char
                highlightedCodeBlock.replace(QChar('\u0000'),
                                             QLatin1String(""));
                str.replace(currentCbPos, next - currentCbPos,
                            highlightedCodeBlock);
                // recalculate next because string has now changed
                next = str.indexOf(type, currentCbPos);
            }
            // move next pos to after the backticks
            next += 3;
            // find the start of the next code block
            currentCbPos = str.indexOf(type, next);
        }
    }
}

QString Md2Html::toHtml(QString str){

    unsigned flags = MD_DIALECT_GITHUB | MD_FLAG_WIKILINKS |
                     MD_FLAG_LATEXMATHSPANS | MD_FLAG_UNDERLINE;
    // we parse the task lists ourselves
    flags &= ~MD_FLAG_TASKLISTS;

    const QSettings settings;
    if (!settings
             .value(QStringLiteral("MainWindow/noteTextView.underline"), true)
             .toBool()) {
        flags &= ~MD_FLAG_UNDERLINE;
    }


    // remove frontmatter from markdown text
    if (str.startsWith(QLatin1String("---"))) {
        str.remove(
            QRegularExpression(QStringLiteral(R"(^---\n.+?\n---\n)"),
                               QRegularExpression::DotMatchesEverythingOption));
    }

    /*CODE HIGHLIGHTING*/
    int cbCount = nonOverlapCount(str, '`');
    if (cbCount % 2 != 0) --cbCount;

    int cbTildeCount = nonOverlapCount(str, '~');
    if (cbTildeCount % 2 != 0) --cbTildeCount;

    // divide by two to get actual number of code blocks
    cbCount /= 2;
    cbTildeCount /= 2;

    highlightCode(str, QStringLiteral("```"), cbCount);
    highlightCode(str, QStringLiteral("~~~"), cbTildeCount);

    const auto data = str.toUtf8();
    if (data.size() == 0) {
        return QLatin1String("");
    }

    QByteArray array;
    const int renderResult =
        md_render_html(data.data(), MD_SIZE(data.size()), &captureHtmlFragment,
                       &array, flags, 0);

    QString result;
    if (renderResult == 0) {
        result = QString::fromUtf8(array);
    } else {
        qWarning() << "MD4C Failure!";
        return QString();
    }
    // for preview
    result =QStringLiteral(
            "<html><head><style>"
            "code{ font-family: FreeMono; }"
            "h1 { margin: 5px 0 20px 0; }"
            "h2, h3 { margin: 10px 0 15px 0; }"
            "table {border-spacing: 0; border-style: solid; border-width: "
            "1px; border-collapse: collapse; margin-top: 0.5em;}"
            "th, td {padding: 2px 5px;}"
            "a { color: #FF9137; text-decoration: none; } %1 %3 %4"
            "</style></head><body class=\"preview\">%1</body></html>")
            .arg(result);
    // remove trailing newline in code blocks
    result.replace(QStringLiteral("\n</code>"), QStringLiteral("</code>"));
    return result;
}


