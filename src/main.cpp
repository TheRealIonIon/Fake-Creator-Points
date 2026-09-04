#include <Geode/Geode.hpp>
#include <Geode/modify/ProfilePage.hpp>
#include <Geode/modify/GJScoreCell.hpp>
#include <Geode/modify/CCTextInputNode.hpp>

using namespace geode::prelude;

static int getCreatorPointCount(GJUserScore* userScore) {
	auto accID = numToString(userScore->m_accountID);

	if (Mod::get()->hasSavedValue(accID)) {
		return Mod::get()->getSavedValue<int>(accID);
	}
	
	return userScore->m_creatorPoints;
}

class EditCreatorPointsPopup : public Popup {
protected:
	int m_accountID = 0;
	std::string m_defaultValue;
	TextInput* m_textBox = nullptr;

	bool init(GJUserScore* userScore) {
		if (!Popup::init(200.f, 150.f)) return false;
		this->setID("EditCreatorPointsPopup");
		m_accountID = userScore->m_accountID;

		auto initVal = numToString(getCreatorPointCount(userScore));
		m_defaultValue = numToString(userScore->m_creatorPoints);

		m_textBox = TextInput::create(100.f, numToString(m_defaultValue));
		this->setTitle("Edit Creator Points Count", "goldFont.fnt", 1.f, 30.f);

		if (initVal != m_defaultValue) m_textBox->setString(initVal);
		m_textBox->setFilter("0123456789");
		m_textBox->setMaxCharCount(9);

		auto saveBtn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create("Apply", "goldFont.fnt", "GJ_button_01.png", 0.8f),
			this, menu_selector(EditCreatorPointsPopup::onSave)
		);

		auto clearBtn = CCMenuItemSpriteExtra::create(
			CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png"),
			this, menu_selector(EditCreatorPointsPopup::onClear)
		);

		clearBtn->setScale(0.85f);
		clearBtn->m_baseScale = 0.85f;

		m_mainLayer->addChildAtPosition(m_textBox, Anchor::Center, { 0.f, 5.f });
		m_buttonMenu->addChildAtPosition(saveBtn, Anchor::Center, { 0.f, -45.f });
		m_buttonMenu->addChildAtPosition(clearBtn, Anchor::TopRight, { -2.5f, -2.5f});

		return true;
	}

	void onSave(CCObject* saveBtn) {
		auto finalValue = m_textBox->getString();
		if (finalValue.empty()) finalValue = m_defaultValue;

		if (finalValue != m_defaultValue) {
			auto intVal = numFromString<int>(finalValue).unwrapOr(0);
			Mod::get()->setSavedValue(numToString(m_accountID), intVal);
		} else Mod::get()->getSaveContainer().erase(numToString(m_accountID));

		auto profilePopup = CCScene::get()->getChildByType<ProfilePage>();
		profilePopup->userInfoChanged(profilePopup->m_score);

		this->keyBackClicked();
	}

	void onClear(CCObject* clearBtn) {
		m_textBox->setString("");
	}

public:
	static EditCreatorPointsPopup* create(GJUserScore* userScore) {
		auto customPopup = new EditCreatorPointsPopup();

		if (customPopup && customPopup->init(userScore)) {
			customPopup->autorelease();
			return customPopup;
		}

		delete customPopup;
		return nullptr;
	}
};

class $modify(CustomProfilePage, ProfilePage) {
	void loadPageFromUserInfo(GJUserScore* userScore) {
		auto oldPts = userScore->m_creatorPoints;
		auto newPts = getCreatorPointCount(userScore);

		userScore->m_creatorPoints = newPts;
		ProfilePage::loadPageFromUserInfo(userScore);

		userScore->m_creatorPoints = oldPts;
		if (userScore->m_accountID == 0) return;

		auto leftMenu = m_mainLayer->getChildByID("left-menu");
		if (!leftMenu || leftMenu->getChildByID("edit-creator-points-button")) return;

		auto hammerSpr = CCSprite::createWithSpriteFrameName("GJ_hammerIcon_001.png");
		auto circleSpr = CircleButtonSprite::create(hammerSpr, CircleBaseColor::Red);
		
		circleSpr->setScale(0.65f);

		auto editBtn = CCMenuItemSpriteExtra::create(
			circleSpr, this,
			menu_selector(CustomProfilePage::onEditCreatorPoints)
		);

		editBtn->setID("edit-creator-points-button");
		leftMenu->addChild(editBtn, -1);
		leftMenu->updateLayout();
	}

	void userInfoChanged(GJUserScore* userScore) {
		auto oldPts = userScore->m_creatorPoints;
		auto newPts = getCreatorPointCount(userScore);

		userScore->m_creatorPoints = newPts;		
		ProfilePage::userInfoChanged(userScore);
		userScore->m_creatorPoints = oldPts;
	}

	void onEditCreatorPoints(CCObject* editBtn) {
		EditCreatorPointsPopup::create(m_score)->show();
	}
};

class $modify(GJScoreCell) {
	void loadFromScore(GJUserScore* userScore) {
		auto oldPts = userScore->m_creatorPoints;
		auto newPts = getCreatorPointCount(userScore);

		userScore->m_creatorPoints = newPts;
		GJScoreCell::loadFromScore(userScore);
		userScore->m_creatorPoints = oldPts;
	}
};

// Forced char limit and filter because of mod menus bypass
// This replicates the vanilla behaviour btw (char filter)
class $modify(CCTextInputNode) {
	void updateLabel(gd::string labelText) {
		if (auto customPopup = CCScene::get()->getChildByID("EditCreatorPointsPopup")) {
			std::string countText = labelText;
			
			for (int charIdx = 0; charIdx < countText.length(); charIdx++) {
				if (!std::isdigit(countText[charIdx])) countText.erase(charIdx--, 1);
			}

			labelText = countText.substr(0, 9);
			this->setString(labelText);
		}

		CCTextInputNode::updateLabel(labelText);
	}
};