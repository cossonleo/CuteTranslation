#include "wordpage.h"
#include "flowlayout.h"
#include "baidutranslate.h"
#include "configtool.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColor>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QPushButton>
#include <QMediaPlayer>
#include <QBuffer>

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent) {

}

ClickableLabel::~ClickableLabel() {}

void ClickableLabel::mousePressEvent(QMouseEvent* ) {
    emit clicked();
}

WordPage::WordPage(QWidget *parent) : QWidget (parent)
{
    initUI();
}

void WordPage::initUI()
{
    // 单词音频播放器
    player = new QMediaPlayer(this);
    player->setVolume(100);
    voiceBuffer = new QBuffer(this);

    // 设置底部颜色
    QPalette palette;
    palette = this->palette();
    palette.setColor(QPalette::Background, QColor(255,255,255));
    this->setPalette(palette);

    // 声音图标
    QPixmap voicePic(":/pic/icons-voice.png");
    voicePic = voicePic.scaled(35, 35, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    titleLabel = new QLabel(this);
    leftAudioLabel = new QLabel(this);
    rightAudioLabel = new QLabel(this);
    mainlayout = new QVBoxLayout;

    titleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    leftAudioLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    rightAudioLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // 单词
    QFont font("Noto Sans CJK SC Regular");
    font.setWeight(QFont::DemiBold);
    titleLabel->setText("Title");
    font.setPixelSize(int(28 * configTool->GetWebPageZoomFactor()));
    titleLabel->setFont(font);
    mainlayout->insertWidget(0, titleLabel);
    mainlayout->insertSpacing(1, 10);
    font.setWeight(QFont::Normal);

    // 声音按钮

    ClickableLabel *leftVoiceButton = new ClickableLabel(this);
    leftVoiceButton->setObjectName("uk"); // 英国
    leftVoiceButton->setFixedHeight(35);
    leftVoiceButton->setContentsMargins(0, 2, 0, 0);
    leftVoiceButton->setPixmap(voicePic);
    connect(leftVoiceButton, &ClickableLabel::clicked, this, &WordPage::onVoiceButtonClicked);


    ClickableLabel *rightVoiceButton = new ClickableLabel(this);
    rightVoiceButton->setObjectName("en"); // 美国
    rightVoiceButton->setFixedHeight(35);
    rightVoiceButton->setContentsMargins(0, 2, 0, 0);
    rightVoiceButton->setPixmap(voicePic);
    connect(rightVoiceButton, &ClickableLabel::clicked, this, &WordPage::onVoiceButtonClicked);

    // 音标
    QHBoxLayout *audioLayout = new QHBoxLayout;
    audioLayout->addWidget(leftAudioLabel);
    audioLayout->addSpacing(10);
    audioLayout->addWidget(leftVoiceButton);
    audioLayout->addStretch();
    leftAudioLabel->setText("Phonetic symbol 1");

    audioLayout->addWidget(rightAudioLabel);
    audioLayout->addSpacing(10);
    audioLayout->addWidget(rightVoiceButton);
    audioLayout->addStretch();
    rightAudioLabel->setText("Phonetic symbol 2");
    mainlayout->insertLayout(2, audioLayout);

    font.setPixelSize(int(20 * configTool->GetWebPageZoomFactor()));

    leftAudioLabel->setFont(font);
    rightAudioLabel->setFont(font);


    palette =  leftAudioLabel->palette();
    palette.setColor(QPalette::WindowText, QColor(88,86,83));

    // init descriptions
    for (int i = 0; i < 10; i++)
    {
        QHBoxLayout *l = new QHBoxLayout;
        l->setContentsMargins(0,5,0,5);

        QLabel *type = new QLabel(this);
        type->setFixedWidth(50);
        font.setPixelSize(int(18 * configTool->GetWebPageZoomFactor()));
        type->setFont(font);
        type->setPalette(palette);
        type->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        type->setTextInteractionFlags(Qt::TextSelectableByMouse);
        QLabel *desc = new QLabel();
        desc->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        desc->setWordWrap(true);
        desc->setTextInteractionFlags(Qt::TextSelectableByMouse);
        desc->setFont(font);
        l->insertWidget(0, type);
        l->insertWidget(1, desc);
        descriptions.append(l);
        mainlayout->addLayout(l); // 12
    }

    // init tags
    tagslayout = new FlowLayout;
    palette = QPalette();
    palette.setColor(QPalette::ColorRole::WindowText, Qt::gray);
    palette.setColor(QPalette::ColorRole::Window, QColor("#F2F1F6"));
    for (int i = 0; i < 10; i++)
    {
        QLabel *tag = new QLabel;
        tag->setFont(font);
        tag->setContentsMargins(5, 3, 5, 3);
        tag->setPalette(palette);
        tag->setAutoFillBackground(true);
        tagslayout->addWidget(tag);
        tags.append(tag);
    }
    mainlayout->insertSpacing(13, 10);
    mainlayout->insertLayout(14, tagslayout);
    mainlayout->addStretch();

    this->setLayout(mainlayout);
    this->adjustSize();


}

void WordPage::updateDescription(const QJsonObject &obj)
{
    for (auto item : descriptions)
    {
        // NOTE THIS
        // 从布局管理器移除元素，元素会显示在它的父控件上，所以这里要把它隐藏起来
        // 布局管理器只控制大小和位置，remove不改变可见性。
        mainlayout->removeItem(item);
        item->itemAt(0)->widget()->hide();
        item->itemAt(1)->widget()->hide();
    }
    for (auto item : tags)
    {
        // tagslayout->removeWidget(item);
        item->hide();
    }

    titleLabel->setText(obj["word_name"].toString());

    // 音标
    QJsonObject symbols = obj["symbols"].toArray().at(0).toObject();
    leftAudioLabel->setText("英 [ " + symbols["ph_en"].toString() + " ]");
    rightAudioLabel->setText("美 [ " + symbols["ph_am"].toString() + " ]");
    // 解释
    QJsonArray parts = symbols["parts"].toArray();
    int i = 0;
    for (i = 0; i < 9 && i < parts.count(); i++)
    {
        QJsonObject item = parts[i].toObject();
        QLabel *type =  reinterpret_cast<QLabel*>(descriptions[i]->itemAt(0)->widget());
        type->setText(item["part"].toString());
        QLabel *desc =  reinterpret_cast<QLabel*>(descriptions[i]->itemAt(1)->widget());
        QString res;
        for (const auto mean : item["means"].toArray())
            res.append(mean.toString() + "，");
        res.remove(res.count() - 1, 1);
        desc->setText(res);
        mainlayout->insertLayout(3 + i, descriptions[i]);
        type->show();
        desc->show();
    }
    // 记忆技巧
    if (obj.contains("memory_skill"))
    {
        QLabel *type =  reinterpret_cast<QLabel*>(descriptions[i]->itemAt(0)->widget());
        type->setText("tip");
        QLabel *desc =  reinterpret_cast<QLabel*>(descriptions[i]->itemAt(1)->widget());
        desc->setText(obj["memory_skill"].toString());
        mainlayout->insertLayout(3 + i, descriptions[i]);
        type->show();
        desc->show();
    }
    i++;
    // 词形变换
    if (obj.contains("exchange"))
    {
        QJsonObject item = obj["exchange"].toObject();
        QLabel *type =  reinterpret_cast<QLabel*>(descriptions[i]->itemAt(0)->widget());
        type->setText("ex");
        QLabel *desc =  reinterpret_cast<QLabel*>(descriptions[i]->itemAt(1)->widget());
        QString res;
        for (const auto &form : item.keys())
        {
            res += form + ": ";
            for (const auto word : item[form].toArray())
                res.append(word.toString() + " ");
            res += "; ";
        }

        desc->setText(res);
        mainlayout->insertLayout(3 + i, descriptions[i]);
        type->show();
        desc->show();
    }

    // 单词标签
    i = 0;
    for (auto item : obj["tags"].toObject()["core"].toArray())
    {
        if (i >= 10 || item.toString().isEmpty())
            break;
        tags[i]->setText(item.toString());
        tags[i++]->show();
    }
    for (auto item : obj["tags"].toObject()["other"].toArray())
    {
        if (i >= 10 || item.toString().isEmpty())
            break;
        tags[i]->setText(item.toString());
        tags[i++]->show();
    }

    mainlayout->update();
    this->update();

}

void WordPage::onVoiceButtonClicked()
{
    if (voiceBuffer->isOpen())
        voiceBuffer->close();
    QString voiceUrl = "https://fanyi.baidu.com/gettts?lan=" + sender()->objectName() +
                       "&text=" + titleLabel->text() + "&spd=3&source=web";
    if (voiceBuffer->property("voiceUrl").toString() != voiceUrl)
    {
        voiceBuffer->setData(BaiduTranslate::instance().getUrlRawContent(voiceUrl));
        voiceBuffer->setProperty("voiceUrl", voiceUrl);
    }
    voiceBuffer->open(QBuffer::ReadOnly);
    player->setMedia(QMediaContent(), voiceBuffer);
    player->play();
}

