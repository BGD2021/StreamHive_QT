#ifndef STREAMLISTWIDGET_H
#define STREAMLISTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>

class StreamItemWidget;

class StreamListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StreamListWidget(QWidget *parent = nullptr);

public slots:
    void addRtspStream(const QString &rtspUrl);
    void clearStreamList();

signals:
    void streamSelected(const QString &streamName, const QString &streamUrl);

private slots:
    void onItemClicked(QListWidgetItem *item);
    void onRefreshButtonClicked();

private:
    void setupUI();
    void addStreamItem(const QString &name, const QString &url, const QString &status = "在线");
    void applyStyles();
    bool parseJsonStreamInfo(const QString &jsonData, QString &name, QString &url, QString &id);
    QString extractStreamName(const QString &rtspUrl);

    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_headerLayout;
    QLabel *m_titleLabel;
    QPushButton *m_refreshButton;
    QListWidget *m_streamList;
    
    // 用于检查重复的URL集合
    QSet<QString> m_existingUrls;
    // 用于检查重复的ID集合
    QSet<QString> m_existingIds;

    // 样式常量
    static const int ITEM_HEIGHT = 80;
    static const int ITEM_MARGIN = 8;
};

class StreamItemWidget : public QFrame
{
    Q_OBJECT

public:
    explicit StreamItemWidget(const QString &name, const QString &url, 
                             const QString &status = "在线", QWidget *parent = nullptr);

    QString getName() const { return m_name; }
    QString getUrl() const { return m_url; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void clicked();

private:
    void setupUI();
    void applyStyles();

    QString m_name;
    QString m_url;
    QString m_status;
    
    QHBoxLayout *m_layout;
    QVBoxLayout *m_infoLayout;
    QLabel *m_nameLabel;
    QLabel *m_urlLabel;
    QLabel *m_statusLabel;
    QLabel *m_arrowLabel;
};

#endif // STREAMLISTWIDGET_H 