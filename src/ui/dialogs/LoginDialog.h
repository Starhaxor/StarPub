#pragma once

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace KeyStar::Ui {

// Spec Section 22 — minimal login. No secrets persisted; session token only.
class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    QString username() const { return m_user; }

private:
    void tryLogin();

    class IconField *m_userEdit;
    class IconField *m_passEdit;
    QLabel *m_apiState;
    QLabel *m_error;
    QPushButton *m_signIn;
    QString m_user;
};

} // namespace KeyStar::Ui
