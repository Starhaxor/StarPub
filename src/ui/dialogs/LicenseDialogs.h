#pragma once

#include "models/Models.h"

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QStringList>

namespace KeyStar::Ui {

// Spec Section 10 — Create License dialog.
class CreateLicenseDialog : public QDialog {
    Q_OBJECT
public:
    explicit CreateLicenseDialog(const QStringList &products, QWidget *parent = nullptr);

    QString product() const;
    int durationDays() const;
    int quantity() const;
    QString notes() const;

private:
    QComboBox *m_product;
    QComboBox *m_duration;
    QSpinBox *m_qty;
    QLineEdit *m_notes;
};

// Spec Section 9 — License details panel (+ Extend inline).
class LicenseDetailsDialog : public QDialog {
    Q_OBJECT
public:
    explicit LicenseDetailsDialog(const License &lic, QWidget *parent = nullptr);

signals:
    void extendRequested(const QString &key, int days);
    void resetHwidRequested(const QString &key);
    void revokeRequested(const QString &key);
};

} // namespace KeyStar::Ui
