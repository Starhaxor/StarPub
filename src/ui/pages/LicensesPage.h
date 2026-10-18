#pragma once

#include "services/Services.h"

#include <QComboBox>
#include <QLabel>
#include <QStringList>
#include <QTableWidget>
#include <QWidget>

namespace KeyStar {
class LicenseService;
class ProductService;
}

namespace KeyStar::Ui {

// Spec Section 8 — License Management: table + search/filter/pagination.
class LicensesPage : public QWidget {
    Q_OBJECT
public:
    explicit LicensesPage(LicenseService *lics, ProductService *prods,
                          QWidget *parent = nullptr);
    void refresh();
    void focusSearch();
    void setProductFilter(const QString &product);

signals:
    void toast(const QString &msg, int kind);
    void loading(bool on);

private:
    void reload();
    void openCreate();
    void openDetails(const QString &key);
    void doExtend(const QString &key);
    void doResetHwid(const QString &key);
    void doRevoke(const QString &key);
    void doDelete(const QString &key);
    void fillTable(const LicensePage &page);

    LicenseService *m_lics;
    ProductService *m_prods;
    LicenseFilter m_filter;
    QStringList m_productNames;

    class SearchBar *m_search;
    QComboBox *m_statusBox;
    QComboBox *m_productBox;
    QTableWidget *m_table;
    QLabel *m_pageLabel;
    QWidget *m_listWrap;
    QWidget *m_emptyWrap = nullptr;
};

} // namespace KeyStar::Ui
