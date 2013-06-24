#ifndef VECTOR_SKIN_H
#define VECTOR_SKIN_H

#include "base_skin.h"

class VectorSkin : public BaseSkin {
public:
  VectorSkin(const QString& skin_root);
  ~VectorSkin();

protected:
  QPixmap* ResizeImage(const QString& s, qreal zoom);
};

#endif // VECTOR_SKIN_H
