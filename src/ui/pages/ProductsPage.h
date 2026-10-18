#pragma once

#include <QLabel>
#include <QWidget>

namespace KeyStar {
class ProductService;
}

namespace KeyStar::Ui {

// Spec Section 11 — Product Management.
class ProductsPage : public QWidget {
    Q_OBJECT
public:
    explicit ProductsPage(ProductService *prods, QWidget *parent = nullptr);
    void refresh();

signals:
    void toast(const QString &msg, int kind);
    void loading(bool on);
    void viewLicenses(const QString &product);

private:
    ProductService *m_prods;
    QWidget *m_cards;
    QLabel *m_count;
};

} // namespace KeyStar::Ui
