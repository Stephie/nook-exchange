module Styles = {
  open Css;
  let cards = style([display(flexBox), flexWrap(wrap)]);
};

let numResultsPerPage = 20;

[@react.component]
let make = () => {
  let (filters, setFilters) =
    React.useState(() =>
      ({orderable: None, hasRecipe: None, category: None}: ItemFilters.t)
    );
  let (pageOffset, setPageOffset) = React.useState(() => 0);
  let filteredItems =
    React.useMemo1(
      () =>
        Item.all->Belt.Array.keep(item =>
          ItemFilters.doesItemMatchFilters(~item, ~filters)
        ),
      [|filters|],
    );
  let numResults = filteredItems->Belt.Array.length;

  <div>
    <div>
      <ItemFilters
        filters
        onChange={filters => {
          setFilters(_ => filters);
          setPageOffset(_ => 0);
        }}
      />
      {if (numResults > 0) {
         <div>
           {React.string(
              "Page "
              ++ string_of_int(pageOffset + 1)
              ++ " of "
              ++ string_of_int(
                   Js.Math.ceil(
                     float_of_int(numResults)
                     /. float_of_int(numResultsPerPage),
                   ),
                 ),
            )}
         </div>;
       } else {
         React.null;
       }}
    </div>
    <div className=Styles.cards>
      {filteredItems
       ->Belt.Array.slice(
           ~offset=pageOffset * numResultsPerPage,
           ~len=numResultsPerPage,
         )
       ->Belt.Array.mapWithIndexU((. i, item) => {
           <ItemCard item key={item.id ++ string_of_int(i)} />
         })
       ->React.array}
    </div>
  </div>;
};