Gem::Specification.new do |s|
  s.name    = "nats-native"
  s.version = "1.4.3"
  s.summary = "NATS client for Ruby"
  s.author  = "Ron Elliott"

  s.files = Dir.glob("ext/**/*.{c,rb}") +
            Dir.glob("lib/**/*.rb")

  s.extensions << "ext/nats/extconf.rb"

  s.add_development_dependency "rake-compiler"
end
