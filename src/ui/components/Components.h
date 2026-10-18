#pragma once

// Reusable Qt components — every screen uses these; no ad-hoc widgets/colors.

#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QWidget>

namespace KeyStar::Ui {

// CStatusBadge — dot + text inside a tinted pill (never color alone).
class StatusBadge : public QWidget {
    Q_OBJECT
public:
    explicit StatusBadge(const QString &status, QWidget *parent = nullptr);
    void setStatus(const QString &status);

private:
    QLabel *m_dot;
    QLabel *m_text;
    QWidget *m_pill;
};

// CMetricCard — small dense cards. cyanValue for the single hero metric.
class MetricCard : public QFrame {
    Q_OBJECT
public:
    explicit MetricCard(const QString &title, QWidget *parent = nullptr,
                        bool cyanValue = false);
    void setValue(const QString &v);
    void setDelta(const QString &d, const QColor &c = QColor());

private:
    QLabel *m_value;
    QLabel *m_delta;
};

// CSectionHeader — title + optional action widget.
class SectionHeader : public QWidget {
    Q_OBJECT
public:
    explicit SectionHeader(const QString &title, QWidget *parent = nullptr,
                           QWidget *action = nullptr);
};

// CIconField — line edit with a leading icon and a focus ring.
class IconField : public QWidget {
    Q_OBJECT
public:
    explicit IconField(const QString &iconName, const QString &placeholder,
                       QWidget *parent = nullptr);
    QLineEdit *edit() const { return m_edit; }
    QString text() const { return m_edit->text(); }
    void setText(const QString &t) { m_edit->setText(t); }
    void setEchoMode(QLineEdit::EchoMode m) { m_edit->setEchoMode(m); }
    void focusEdit();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    QLineEdit *m_edit;
};

// CSearchBar — IconField with clear button + debounced signal.
class SearchBar : public IconField {
    Q_OBJECT
public:
    explicit SearchBar(const QString &placeholder, QWidget *parent = nullptr);
    void focusSearch();
signals:
    void debounced(const QString &text);

private:
    QTimer m_timer;
};

// CToast host — short-lived, never covers UI.
class Toast : public QWidget {
    Q_OBJECT
public:
    enum class Kind { Success, Warning, Error };
    static void show(QWidget *host, const QString &msg, Kind k = Kind::Success);

private:
    explicit Toast(QWidget *host, const QString &msg, Kind k);
};

// CLoadingOverlay — dim + centered card, shown during requests.
class LoadingOverlay : public QWidget {
    Q_OBJECT
public:
    explicit LoadingOverlay(QWidget *host, const QString &text = "Loading");
    void finish();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    QLabel *m_label;
    QString m_base;
    int m_dots = 0;
};

// Confirm dialogs — destructive ops always confirm; delete types DELETE.
bool confirmAction(QWidget *parent, const QString &title, const QString &body,
                   const QString &okText, bool danger = false);
bool confirmDelete(QWidget *parent, const QString &target);

// Factories.
QPushButton *primaryButton(const QString &text, QWidget *parent = nullptr,
                           const QString &icon = {});
QPushButton *ghostButton(const QString &text, QWidget *parent = nullptr,
                         const QString &icon = {});
QPushButton *dangerButton(const QString &text, QWidget *parent = nullptr,
                          const QString &icon = {});
QPushButton *linkButton(const QString &text, QWidget *parent = nullptr);
QPushButton *iconButton(const QString &icon, const QString &tip,
                        QWidget *parent = nullptr, bool danger = false);
QLineEdit *field(QWidget *parent = nullptr);
QLabel *monoLabel(const QString &text, QWidget *parent = nullptr);
QLabel *secondaryLabel(const QString &text, QWidget *parent = nullptr);
QLabel *captionLabel(const QString &text, QWidget *parent = nullptr);
QFrame *divider(QWidget *parent = nullptr);
QWidget *emptyState(const QString &title, const QString &body, QWidget *action = nullptr,
                    const QString &icon = "inbox");

} // namespace KeyStar::Ui
