@genType
type notRecursive = int

module M = {
  @genType
  type notRecursive = array<notRecursive>

  @genType
  type rec recursive = {self: recursive}
}
