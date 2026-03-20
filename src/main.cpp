#include <SFML/Graphics.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/Window.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

enum class ShapeId
{
  Circle,
  Square,
  Triangle,
  Rectangle
};

class Config
{
 public:
  Config()
  {
    // В конструкторе могли бы загружать из файла настройки
    // или определять размер окна в зависимости от размера экрана и т.п.

    sfml_settings_.antiAliasingLevel = antialiasing_level_;
  }

  const sf::ContextSettings& sfml_settings() const noexcept
  {
    return sfml_settings_;
  }
  const std::string& window_title() const noexcept { return window_title_; }
  //SFML требует unsigned int в качестве размера стороны окна
  unsigned int window_width() const noexcept { return window_width_; }
  unsigned int window_height() const noexcept { return window_height_; }
  sf::Color background_color() const noexcept { return background_color_; }
  sf::Color shape_color() const noexcept { return shape_color_; }
  float default_size() const noexcept { return default_size_; }

 private:
  sf::ContextSettings sfml_settings_;
  std::string window_title_{"Display shapes"};
  unsigned int window_width_ = 800;
  unsigned int window_height_ = 600;
  unsigned int antialiasing_level_ = 8;
  //цвет RGB + alpha
  sf::Color background_color_{sf::Color{82, 87, 110, 255}};
  sf::Color shape_color_{sf::Color{200, 197, 189, 255}};
  //Характерный размер по умолчанию для фигур
  float default_size_ = 100.f;
};

sf::RenderWindow create_window(const Config& config)
{
  return sf::RenderWindow{
      sf::VideoMode({config.window_width(), config.window_height()}),
      config.window_title(), sf::State::Windowed, config.sfml_settings()};
}

class Shape
{
 public:
  virtual ~Shape() = default;
  virtual void draw(sf::RenderTarget& target) const = 0;
};

class Circle : public Shape
{
 public:
  Circle(sf::Vector2f pos, float radius, sf::Color color,
         std::size_t num_points = 30)
  {
    shape_.setRadius(radius);
    shape_.setPointCount(num_points);
    shape_.setOrigin(sf::Vector2f{radius, radius});
    shape_.setPosition(pos);
    shape_.setFillColor(color);
  }

  virtual void draw(sf::RenderTarget& target) const override
  {
    target.draw(shape_);
  }

 private:
  sf::CircleShape shape_;
};

class Square : public Shape
{
 public:
  Square(sf::Vector2f pos, float side_length, sf::Color color)
  {
    shape_.setSize(sf::Vector2f{side_length, side_length});
    sf::Vector2f origin{side_length / 2.f, side_length / 2.f};
    shape_.setOrigin(origin);
    shape_.setPosition(pos);
    shape_.setFillColor(color);
  }

  virtual void draw(sf::RenderTarget& target) const override
  {
    target.draw(shape_);
  }

 private:
  sf::RectangleShape shape_;
};

class Triangle : public Shape
{
 public:
  Triangle(sf::Vector2f pos, float side_length, sf::Color color)
  {
    const float radius = side_length / std::sqrt(3.f);
    shape_.setRadius(radius);
    shape_.setPointCount(3);

    //Определяем реальные размеры фигуры и находим центр
    const auto local_bounds = shape_.getLocalBounds();
    const float origin_x = local_bounds.position.x + local_bounds.size.x / 2.f;
    const float origin_y = local_bounds.position.y + local_bounds.size.y / 2.f;
    shape_.setOrigin(sf::Vector2f{origin_x, origin_y});

    shape_.setPosition(pos);
    shape_.setFillColor(color);
  }

  virtual void draw(sf::RenderTarget& target) const override
  {
    target.draw(shape_);
  }

 private:
  // В библиотке нет класса для треугольника -> аппроксимируем
  // кругом с числом точек 3. Если нужен не равносторонний
  // треугольник, то нужен будет sf::VertexArray с явным заданием
  // всех трех координат вершин
  sf::CircleShape shape_;
};

class Rectangle : public Shape
{
 public:
  Rectangle(sf::Vector2f pos, sf::Vector2f side_lengths, sf::Color color)
  {
    const auto width = side_lengths.x;
    const auto height = side_lengths.y;
    shape_.setSize(sf::Vector2f{width, height});
    sf::Vector2f origin{width / 2.f, height / 2.f};
    shape_.setOrigin(origin);
    shape_.setPosition(pos);
    shape_.setFillColor(color);
  }

  virtual void draw(sf::RenderTarget& target) const override
  {
    target.draw(shape_);
  }

 private:
  sf::RectangleShape shape_;
};

std::unique_ptr<Shape> create_shape(ShapeId shape_id, sf::Vector2f pos,
                                    float size, sf::Color color)
{
  switch (shape_id)
  {
    case ShapeId::Circle:
    {
      const float radius = size / 2.f;
      return std::make_unique<Circle>(pos, radius, color);
    }
    case ShapeId::Square:
    {
      return std::make_unique<Square>(pos, size, color);
    }
    case ShapeId::Triangle:
    {
      return std::make_unique<Triangle>(pos, size, color);
    }
    case ShapeId::Rectangle:
    {
      const float width = size;
      //Зафиксированное соотношение сторон 4/3
      constexpr float aspect_ratio = 4. / 3.;
      const float height = size / aspect_ratio;
      const sf::Vector2f sizes{width, height};
      return std::make_unique<Rectangle>(pos, sizes, color);
    }
    default:
    {
      throw std::invalid_argument("Неподдерживаемый тип фигуры");
    }
  }
  //unreachable, return nullptr to suppress compiler warning
  return nullptr;
}

ShapeId str_to_id(const std::string& str)
{
  if (str == "circle")
  {
    return ShapeId::Circle;
  }
  if (str == "triangle")
  {
    return ShapeId::Triangle;
  }
  if (str == "square")
  {
    return ShapeId::Square;
  }
  if (str == "rectangle")
  {
    return ShapeId::Rectangle;
  }

  throw std::invalid_argument("Неподдерживаемый тип фигуры: " + str);
  //unreachable, return 0 to suppress compiler warning
  return static_cast<ShapeId>(0);
}

std::vector<std::pair<ShapeId, int>> parse_args(int argc, char* argv[])
{
  if (argc < 2)
  {
    throw std::invalid_argument("Нет входных аргументов");
  }
  if ((argc - 1) % 2 != 0)
  {
    throw std::invalid_argument(
        "Ключи должны идти в формате <Тип фигуры> <число>");
  }

  const int num_pairs = (argc - 1) / 2;
  std::vector<std::pair<ShapeId, int>> input_data;
  input_data.reserve(num_pairs);

  std::string buf;
  for (int i = 1; i < argc; i += 2)
  {
    buf.assign(argv[i]);
    //Переводим все в нижний регистр
    std::transform(buf.cbegin(), buf.cend(), buf.begin(),
                   [](unsigned char ch) { return std::tolower(ch); });
    const auto shape_id = str_to_id(buf);

    buf.assign(argv[i + 1]);
    int count{};
    try
    {
      count = std::stoi(buf);
    }
    catch (...)
    {
      throw std::invalid_argument("Не удалось преобразовать " + buf +
                                  " в число");
    }

    if (count < 1)
    {
      throw std::invalid_argument("Число фигур должно быть положительным");
    }

    input_data.emplace_back(std::pair{shape_id, count});
  }

  return input_data;
}

void main_loop(const Config& config, sf::RenderWindow& window,
               const std::vector<std::unique_ptr<Shape>>& shapes)
{
  while (window.isOpen())
  {
    while (const std::optional event = window.pollEvent())
    {
      if (event->is<sf::Event::Closed>()) window.close();
    }

    window.clear(config.background_color());

    for (const auto& shape_ptr : shapes)
    {
      shape_ptr->draw(window);
    }
    window.display();
  }
}

std::vector<std::unique_ptr<Shape>> create_shapes(
    const Config& config,
    const std::vector<std::pair<ShapeId, int>>& input_data)
{
  std::vector<std::unique_ptr<Shape>> shapes;
  int num_shapes = 0;
  for (const auto& [shape_id, count] : input_data)
  {
    num_shapes += count;
  }
  shapes.reserve(num_shapes);

  float curr_x_pos = config.default_size();
  float curr_y_pos = config.default_size();
  const float offset_x = config.default_size() * 2.f;
  for (const auto& [shape_id, count] : input_data)
  {
    for (int i = 0; i < count; ++i)
    {
      const auto pos = sf::Vector2f{curr_x_pos, curr_y_pos};
      shapes.emplace_back(create_shape(shape_id, pos, config.default_size(),
                                       config.shape_color()));
      curr_x_pos += offset_x;
    }
  }

  return shapes;
}

int main(int argc, char* argv[])
{
#if defined(_WIN32)
  setlocale(LC_ALL, ".UTF-8");
  setlocale(LC_NUMERIC, "C");
  SetConsoleOutputCP(CP_UTF8);
#endif

  try
  {
    auto input_data = parse_args(argc, argv);

    Config config;
    auto window = create_window(config);
    auto shapes = create_shapes(config, input_data);

    main_loop(config, window, shapes);
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cerr << "Неизвестная ошибка" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
